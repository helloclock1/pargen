#include "BNFParser.h"

#include "Entities.h"
#include "Helpers.h"

GrammarParserError::GrammarParserError(const std::string &msg, size_t line)
    : msg_(ConstructMessage(msg, line)) {
}

const char *GrammarParserError::what() const noexcept {
    return msg_.c_str();
}

std::string GrammarParserError::ConstructMessage(
    const std::string &msg, size_t line
) {
    return "[" + std::to_string(line) + "]: " + msg;
}

GrammarParser::GrammarParser(std::unique_ptr<std::istream> in)
    : in_(std::move(in)) {
}

void GrammarParser::Parse() {
    while (!PeekAt(EOF)) {
        // invariant: here in_ always peeks at line beginning
        ++line_;
        SkipWS();
        ParseLine();
        GetChar();
    }
    if (g_.rules_.empty()) {
        throw GrammarParserError("Empty grammar", line_);
    }
    NonTerminal first_rule = g_.rules_[0].lhs;
    g_.rules_.insert(g_.rules_.cbegin(), Rule{NonTerminal{"S'"}, {first_rule}});
    g_.tokens_.insert(first_rule);
    g_.tokens_.insert(T_EOF);
}

const Grammar &GrammarParser::Get() const {
    return g_;
}

int GrammarParser::GetChar() {
    return in_->get();
}

int GrammarParser::GetChar(int expected) {
    int c = GetChar();
    if (c != expected) {
        throw GrammarParserError(
            "Unexpected character: `" + (c != -1 ? std::string(1, c) : "EOF") +
                "` != `" + std::string(1, expected) + "`",
            line_
        );
    }
    return c;
}

int GrammarParser::Peek() const {
    return in_->peek();
}

bool GrammarParser::PeekAt(int c) const {
    return Peek() == c;
}

void GrammarParser::SkipWS() {
    while (std::isspace(Peek()) && !PeekAt('\n')) {
        GetChar();
    }
}

void GrammarParser::ParseLine() {
    if (PeekAt('\n')) {
        GetChar();
        return;
    }
    Token lhs = ParseToken();
    SkipWS();
    GetChar('=');
    SkipWS();
    size_t rule_number = 1;
    if (std::holds_alternative<Terminal>(lhs)) {
        Terminal t = std::get<Terminal>(lhs);
        if (t.name_ == "IGNORE") {
            ParseIgnore();
            return;
        }
        if (t.repr_.empty()) {
            throw GrammarParserError(
                "Can't assign a regex to a quote terminal; try removing "
                "surrounding quotes on LHS",
                line_
            );
        }
        std::string regex;
        while (!(PeekAt('\n') || PeekAt(EOF))) {
            regex += GetChar();
        }
        size_t last_non_space = regex.find_last_not_of(' ');
        if (last_non_space != std::string::npos) {
            regex = regex.substr(0, last_non_space + 1);
        }
        g_.tokens_.insert(Terminal{t.name_, regex});
    } else if (std::holds_alternative<NonTerminal>(lhs)) {
        NonTerminal nt_lhs = std::get<NonTerminal>(lhs);
        while (!(PeekAt('\n') || PeekAt(EOF))) {
            SkipWS();
            Production prod = ParseRule();
            if (prod.empty()) {
                throw GrammarParserError(
                    "Empty production instead of rule #" +
                        std::to_string(rule_number) +
                        "; try removing duplicate `|`",
                    line_
                );
            } else {
                g_.rules_.push_back(Rule{nt_lhs, prod});
            }
            SkipWS();
            if (PeekAt('|')) {
                GetChar();
            }
            ++rule_number;
        }
    } else {
        throw GrammarParserError("Unknown token type encountered", line_);
    }
}

std::vector<Token> GrammarParser::ParseRule() {
    std::vector<Token> production;
    bool has_epsilon = false;
    while (!(PeekAt('\n') || PeekAt(EOF) || PeekAt('|'))) {
        Token token = ParseToken();
        bool is_eps = false;
        if (std::holds_alternative<Terminal>(token)) {
            Terminal t = std::get<Terminal>(token);
            if (!t.repr_.empty()) {
                if (t.name_ == "EPSILON") {
                    is_eps = true;
                    has_epsilon = true;
                    token = Terminal{""};
                } else {
                    bool found = false;
                    for (const Token &token : g_.tokens_) {
                        if (std::holds_alternative<Terminal>(token) &&
                            std::get<Terminal>(token).name_ == t.name_) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        throw GrammarParserError(
                            "Unknown terminal encountered: " + t.name_ +
                                "; if the token is defined after this line, "
                                "try moving it before the current line",
                            line_
                        );
                    }
                }
            }
        }
        production.push_back(token);
        if (!is_eps) {
            g_.tokens_.insert(token);
        }
        SkipWS();
    }
    if (has_epsilon && production.size() != 1) {
        throw GrammarParserError(
            "Epsilon can only be used in a single-token production; try "
            "getting rid of unnecessary epsilon productions",
            line_
        );
    }
    return production;
}

Token GrammarParser::ParseToken() {
    if (PeekAt('<')) {
        GetChar();
        NonTerminal result = NonTerminal{ParseName()};
        char c = GetChar();
        if (c != '>') {
            throw GrammarParserError("Unterminated `<`", line_);
        }
        return result;
    } else if (PeekAt('\'') || PeekAt('"')) {
        return ParseQuoteTerminal();
    } else if (std::isalpha(Peek())) {
        return Terminal{ParseName(), " "};
    }
    throw GrammarParserError(
        "Unknown token, peeking at `" +
            (PeekAt(EOF) ? "EOF" : std::string(1, Peek())) + "`",
        line_
    );
}

Terminal GrammarParser::ParseQuoteTerminal() {
    char init = GetChar();
    std::string lexeme;
    while (!(PeekAt(init) || PeekAt('\n') || PeekAt(EOF))) {
        lexeme += GetChar();
    }
    if (!GetChar(init)) {
        throw GrammarParserError("Unterminated quote terminal", line_);
    }
    return Terminal{lexeme};
}

std::string GrammarParser::ParseName() {
    std::string result;
    while ((std::isalnum(Peek()) || PeekAt('_')) && in_->peek() != '\n') {
        result += GetChar();
    }
    return result;
}

void GrammarParser::ParseIgnore() {
    std::string regex;
    while (!(PeekAt('\n') || PeekAt(EOF))) {
        regex += GetChar();
    }
    g_.ignored_.push_back(regex);
}

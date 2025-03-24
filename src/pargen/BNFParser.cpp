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
        ++line_;
        SkipWS();
        ParseLine();
        GetChar();
    }

    Verify();
    Augment();
}

const Grammar &GrammarParser::Get() const {
    return g_;
}

void GrammarParser::ThrowError(const std::string &msg) {
    throw GrammarParserError(msg, line_);
}

int GrammarParser::GetChar() {
    return in_->get();
}

int GrammarParser::GetChar(int expected) {
    int c = GetChar();
    if (c != expected) {
        ThrowError(
            "Unexpected character: `" + (c != -1 ? std::string(1, c) : "EOF") +
            "` != `" + std::string(1, expected) + "`"
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
    if (PeekAt('\n') || PeekAt(EOF)) {
        return;
    }

    Token lhs = ParseToken();
    SkipWS();

    GetChar('=');
    SkipWS();
    if (PeekAt('\n') || PeekAt(EOF)) {
        ThrowError("Empty production");
    }

    size_t rule_number = 1;
    if (IsTerminal(lhs)) {
        Terminal t = std::get<Terminal>(lhs);
        if (t.name_ == "IGNORE") {
            ParseIgnore();
            return;
        } else if (t.name_ == "EPSILON") {
            std::cerr << "Warning: changing EPSILON is not allowed, ignoring "
                         "declaration on line "
                      << line_ << std::endl;
        }
        if (t.IsQuote()) {
            ThrowError(
                "Can't assign a regex to a quote terminal; try removing "
                "surrounding quotes on LHS"
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
    } else if (IsNonTerminal(lhs)) {
        NonTerminal nt_lhs = std::get<NonTerminal>(lhs);
        while (!(PeekAt('\n') || PeekAt(EOF))) {
            SkipWS();
            Production prod = ParseProduction();
            if (prod.empty()) {
                std::cerr << "Warning: empty production on line " << line_
                          << std::endl;
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
        ThrowError("Unknown token type encountered");
    }
}

Production GrammarParser::ParseProduction() {
    std::vector<Token> production;
    bool has_epsilon = false;
    while (!(PeekAt('\n') || PeekAt(EOF) || PeekAt('|'))) {
        Token token = ParseToken();
        bool is_eps = false;
        if (IsTerminal(token)) {
            Terminal t = std::get<Terminal>(token);
            if (t.IsRegex()) {
                if (t.name_ == "EPSILON") {
                    is_eps = true;
                    has_epsilon = true;
                    token = EPSILON;
                } else {
                    bool found = false;
                    for (const Token &defined_token : g_.tokens_) {
                        if (IsTerminal(defined_token) &&
                            std::get<Terminal>(defined_token).name_ ==
                                t.name_) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        ThrowError(
                            "Unknown terminal encountered: " + t.name_ +
                            "; if the token is defined after this line, "
                            "try moving it before the current line"
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
        ThrowError(
            "Epsilon can only be used in a single-token production; try "
            "getting rid of unnecessary epsilon productions"
        );
    }

    return production;
}

Token GrammarParser::ParseToken() {
    if (PeekAt('<')) {
        GetChar();
        NonTerminal token = NonTerminal{ParseName()};
        if (GetChar() != '>') {
            ThrowError("Unterminated `<`");
        }
        return token;
    } else if (PeekAt('\'') || PeekAt('"')) {
        return ParseQuoteTerminal();
    } else if (std::isalpha(Peek())) {
        return Terminal{ParseName(), " "};
    }

    ThrowError(
        "Unknown token, peeking at `" +
        (PeekAt(EOF) ? "EOF" : std::string(1, Peek())) + "`"
    );
    return Token{};
}

Terminal GrammarParser::ParseQuoteTerminal() {
    char init = GetChar();
    std::string lexeme;
    while (!(PeekAt(init) || PeekAt('\n') || PeekAt(EOF))) {
        lexeme += GetChar();
    }
    if (GetChar() != init) {
        ThrowError("Unterminated quote terminal");
    }

    return Terminal{lexeme};
}

std::string GrammarParser::ParseName() {
    std::string name;
    while ((std::isalnum(Peek()) || PeekAt('_')) && in_->peek() != '\n') {
        name += GetChar();
    }

    return name;
}

void GrammarParser::ParseIgnore() {
    std::string regex;
    while (!(PeekAt('\n') || PeekAt(EOF))) {
        regex += GetChar();
    }

    g_.ignored_.push_back(regex);
}

void GrammarParser::Verify() {
    if (g_.rules_.empty()) {
        ThrowError("Empty grammar");
    }

    line_ = 1;
    for (const Rule &rule : g_.rules_) {
        for (const Token &token : rule.prod) {
            if (IsTerminal(token)) {
                continue;
            }

            bool found = false;
            for (const Rule &other : g_.rules_) {
                if (std::get<NonTerminal>(token) == other.lhs) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                ThrowError(
                    "Encountered an undefined non-terminal " +
                    std::get<NonTerminal>(token).name_
                );
            }
        }
        ++line_;
    }
}

void GrammarParser::Augment() {
    NonTerminal first_rule = g_.rules_[0].lhs;
    g_.rules_.insert(g_.rules_.cbegin(), Rule{NonTerminal{"S'"}, {first_rule}});
    g_.tokens_.insert({first_rule, T_EOF});
}

#include "BNFParser.h"

#include <cctype>
#include <stdexcept>
#include <variant>

#include "Entities.h"

GrammarParser::GrammarParser(std::istream *in) : in_(in) {
}

void GrammarParser::Parse() {
    while (in_->peek() != -1) {
        // invariant: here in_ always peeks at line beginning
        SkipWS();
        ParseLine();
        GetChar();
    }
}

const Grammar &GrammarParser::Get() const {
    return g_;
}

int GrammarParser::GetChar() {
    return in_->get();
}

int GrammarParser::GetChar(int expected) {
    int c = in_->get();
    if (c != expected) {
        throw std::runtime_error("Unexpected character: `" + std::string(1, c) +
                                 "` != `" + std::string(1, expected) + "`");
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
    while (std::isspace(in_->peek()) && in_->peek() != '\n') {
        in_->get();
    }
}

void GrammarParser::ParseLine() {
    Token lhs = ParseToken();
    SkipWS();
    GetChar('=');
    SkipWS();
    if (std::holds_alternative<Terminal>(lhs)) {
        Terminal t = std::get<Terminal>(lhs);
        std::string regex;
        while (!(PeekAt('\n') || PeekAt(EOF))) {
            char c = GetChar();
            regex += c;
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
                char response;
                do {
                    std::cout << "Encountered an empty production #k on line "
                                 "n, did you want "
                                 "it to be an empty string? If not, the "
                                 "production will "
                                 "be ignored [y/n]: ";
                    std::cin >> response;
                } while (!(response == 'y' || response == 'n'));
                if (response == 'y') {
                    prod = {Terminal{""}};
                }
            } else {
                g_.rules_.push_back(Rule{nt_lhs, prod});
            }
            SkipWS();
            if (PeekAt('|')) {
                GetChar();
            }
        }
    } else {
        throw std::logic_error("Unknown token type encountered");
    }
}

std::vector<Token> GrammarParser::ParseRule() {
    std::vector<Token> production;
    while (!(PeekAt('\n') || PeekAt(EOF) || PeekAt('|'))) {
        Token token = ParseToken();
        if (std::holds_alternative<Terminal>(token)) {
            Terminal t = std::get<Terminal>(token);
            if (!t.repr_.empty() && t.name_ == "EPSILON") {
                token = Terminal{""};
            }
        }
        production.push_back(token);
        g_.tokens_.insert(token);
        SkipWS();
    }
    return production;
}

Token GrammarParser::ParseToken() {
    if (PeekAt('<')) {
        GetChar();
        NonTerminal result = NonTerminal{ParseName()};
        char c = in_->get();
        if (c != '>') {
            throw std::runtime_error("Unterminated `<`");
        }
        return result;
    } else if (PeekAt('\'') || PeekAt('"')) {
        return ParseQuoteTerminal();
    } else if (std::isalpha(Peek())) {
        return Terminal{ParseName(), " "};
    }
    throw std::runtime_error("Unknown token, peeking at `" +
                             std::string(1, Peek()) + "`");
}

Terminal GrammarParser::ParseQuoteTerminal() {
    char init = in_->get();
    std::string lexeme;
    while (in_->peek() != init && in_->peek() != '\n') {
        lexeme += in_->get();
    }
    if (in_->get() != init) {
        throw std::runtime_error("Unterminated quote terminal");
    }
    return Terminal{lexeme};
}

std::string GrammarParser::ParseName() {
    std::string result;
    while ((std::isalnum(Peek()) || PeekAt('_')) && in_->peek() != '\n') {
        result += in_->get();
    }
    return result;
}

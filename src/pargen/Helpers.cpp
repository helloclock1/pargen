#include "Helpers.h"

bool IsTerminal(const Token &token) {
    return std::holds_alternative<Terminal>(token);
}

bool IsNonTerminal(const Token &token) {
    return std::holds_alternative<NonTerminal>(token);
}

std::string QualName(Token token) {
    if (IsTerminal(token)) {
        Terminal t = std::get<Terminal>(token);
        if (t.repr_.empty()) {
            return "T_" + t.name_;
        } else {
            return "R_" + t.name_;
        }
    } else {
        return "NT_" + std::get<NonTerminal>(token).name_;
    }
}
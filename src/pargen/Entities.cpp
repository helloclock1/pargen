#include "Entities.h"

bool Terminal::operator==(const Terminal &other) const {
    if (repr_.empty() + other.repr_.empty() == 1) {
        return false;
    }
    return name_ == other.name_;
}

bool Terminal::operator!=(const Terminal &other) const {
    return !(*this == other);
}

bool NonTerminal::operator==(const NonTerminal &other) const {
    return name_ == other.name_;
}

bool NonTerminal::operator!=(const NonTerminal &other) const {
    return name_ != other.name_;
}

bool operator<(const Token &a, const Token &b) {
    if (std::holds_alternative<Terminal>(a) &&
        std::holds_alternative<Terminal>(b)) {
        return std::get<Terminal>(a).name_ < std::get<Terminal>(b).name_;
    } else if (std::holds_alternative<NonTerminal>(a) &&
               std::holds_alternative<NonTerminal>(b)) {
        return std::get<NonTerminal>(a).name_ < std::get<NonTerminal>(b).name_;
    } else if (std::holds_alternative<Terminal>(a) &&
               std::holds_alternative<NonTerminal>(b)) {
        return true;
    } else if (std::holds_alternative<NonTerminal>(a) &&
               std::holds_alternative<Terminal>(b)) {
        return false;
    }
    return true;
}
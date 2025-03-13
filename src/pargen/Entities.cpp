#include "Entities.h"

bool Terminal::operator==(const Terminal &other) const {
    if (repr_.empty() != other.repr_.empty()) {
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

// TODO(helloclock): rewrite to visitor pattern?
bool operator<(const Token &a, const Token &b) {
    if (std::holds_alternative<Terminal>(a) &&
        std::holds_alternative<Terminal>(b)) {
        Terminal at = std::get<Terminal>(a);
        Terminal bt = std::get<Terminal>(b);
        return at.name_ < bt.name_ ||
               (at.name_ == bt.name_ && at.repr_ < bt.repr_);
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
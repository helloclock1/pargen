#include "Entities.h"

bool Terminal::IsQuote() const {
    return repr_.empty();
}

bool Terminal::IsRegex() const {
    return !repr_.empty();
}

bool Terminal::operator==(const Terminal &other) const {
    if (repr_.empty() != other.repr_.empty()) {
        return false;
    }
    return name_ == other.name_;
}

bool Terminal::operator!=(const Terminal &other) const {
    return !(*this == other);
}

bool Terminal::operator<(const Terminal &other) const {
    if (repr_.empty() != other.repr_.empty()) {
        return repr_.empty() < other.repr_.empty();
    }
    return name_ < other.name_;
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
        return std::tie(at.name_, at.repr_) < std::tie(bt.name_, bt.repr_);
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
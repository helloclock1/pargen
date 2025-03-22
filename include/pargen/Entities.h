#pragma once

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct Terminal {
    std::string name_;
    std::string repr_ = "";
    bool operator==(const Terminal &other) const;
    bool operator!=(const Terminal &other) const;
    bool operator<(const Terminal &other) const;
};

struct NonTerminal {
    std::string name_;
    bool operator==(const NonTerminal &other) const;
    bool operator!=(const NonTerminal &other) const;
};

using Token = std::variant<Terminal, NonTerminal>;

namespace std {
template <>
struct hash<Terminal> {
    size_t operator()(const Terminal &t) const {
        if (t.repr_.empty()) {
            return hash<string>()("t" + t.name_);
        } else {
            return hash<string>()("r" + t.name_);
        }
    }
};

template <>
struct hash<NonTerminal> {
    size_t operator()(const NonTerminal &nt) const {
        return hash<string>()("nt" + nt.name_);
    }
};

template <>
struct hash<Token> {
    size_t operator()(const Token &t) const {
        if (std::holds_alternative<Terminal>(t)) {
            return hash<Terminal>()(std::get<Terminal>(t));
        } else {
            return hash<NonTerminal>()(std::get<NonTerminal>(t));
        }
    }
};
};  // namespace std

bool operator<(const Token &a, const Token &b);

using Production = std::vector<Token>;

struct Rule {
    NonTerminal lhs;
    Production prod;
};

struct Grammar {
    std::vector<Rule> rules_;
    std::set<Token> tokens_;
    std::vector<std::string> ignored_;

    Rule &operator[](size_t i) {
        return rules_[i];
    }

    const Rule &operator[](size_t i) const {
        return rules_[i];
    }
};

using FirstSets = std::map<Token, std::set<Terminal>>;
using FollowSets = std::unordered_map<NonTerminal, std::set<Terminal>>;

enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

struct Action {
    ActionType type_ = ActionType::ERROR;
    size_t value_ = 0;
};

using ActionTable = std::vector<std::unordered_map<std::string, Action>>;
using GotoTable =
    std::unordered_map<size_t, std::unordered_map<NonTerminal, size_t>>;
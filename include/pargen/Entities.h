/**
 * @file Entities.h
 * @brief Provides structures for representing everything needed for generating
 * a parser: terminals, nonterminals, tokens, rules, grammar and parser tables.
 * @author Vadim Melnikov
 * @version 1.0
 */
#pragma once

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

/**
 * @struct Terminal
 * @brief Represents a terminal symbol in a grammar.
 * @details A terminal symbol is a literal string that can appear in a sentence
 * in the user-given grammar.
 */
struct Terminal {
    /**
     * @brief Stores the name of the terminal symbol.
     */
    std::string name_;
    /**
     * @brief Stores the literal representation of the terminal symbol.
     * @details  When parsing the grammar, contains something (the regex
     * expression if encountered a regex terminal definition, a single
     * whitespace if it is encountered in a rule) if it is a regex terminal,
     * contains nothing if it is a quote terminal.
     */
    std::string repr_ = "";

    bool IsQuote() const;
    bool IsRegex() const;

    /**
     * @brief Compares two terminals for equality.
     * @details If exactly one terminal has a non-empty `repr_` field, returns
     * false. Otherwise checks for equality of the names.
     */
    bool operator==(const Terminal &other) const;
    bool operator!=(const Terminal &other) const;
    /**
     * @brief Compares two terminals for ordering.
     * @details Arranges terminals in lexicographical order {repr_.empty(),
     * name_}.
     */
    bool operator<(const Terminal &other) const;
};

/**
 * @struct NonTerminal
 * @brief Represents a non-terminal symbol in a grammar.
 * @details A non-terminal symbol is a symbol that can be replaced by a sequence
 * of non-terminals and/or terminals in the user-given grammar.
 */
struct NonTerminal {
    /**
     * @brief Stores the name of the non-terminal symbol.
     */
    std::string name_;
    bool operator==(const NonTerminal &other) const;
    bool operator!=(const NonTerminal &other) const;
};

/**
 * @brief Alias for a variant of a terminal or a non-terminal.
 */
using Token = std::variant<Terminal, NonTerminal>;

/*
 * The following code is needed to make structures like `std::unordered_map`
 * work with `Terminal`, `NonTerminal` and `Token` types.
 */
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

/**
 * @brief Compares two tokens for ordering.
 * @details If both tokens are of same type, compares them using their
 * comparator. Otherwise compares tokens using lexicographical ordering
 * {this.IsTerminal(), this.IsNonTerminal()}.
 */
bool operator<(const Token &a, const Token &b);

/**
 * @brief Alias for a vector of tokens representing a RHS production rule.
 */
using Production = std::vector<Token>;

/**
 * @struct Rule
 * @brief Represents an entire grammar rule.
 */
struct Rule {
    /**
     * @brief Stores the LHS of the rule.
     */
    NonTerminal lhs;
    /**
     * @brief Stores a single production of the rule.
     */
    Production prod;
};

/**
 * @struct Grammar
 * @brief Represents the parsed user-defined grammar.
 */
struct Grammar {
    /**
     * @brief Stores a list of all rules in the grammar.
     */
    std::vector<Rule> rules_;
    /**
     * @brief Stores a list of symbols of the grammar, both defined and
     * encountered while parsing.
     */
    std::set<Token> tokens_;
    /**
     * @brief Stores a list of all symbols that shall be ignored by the lexer
     * generated in the future.
     */
    std::vector<std::string> ignored_;

    /**
     * @brief Quality of life function for accessing a certain rule.
     * @param i Index of the rule in the grammar.
     * @return Reference to the rule at index i.
     */
    Rule &operator[](size_t i) {
        return rules_[i];
    }

    /**
     * @brief Quality of life function for accessing a certain rule.
     * @param i Index of the rule in the grammar.
     * @return Const reference to the rule at index i.
     */
    const Rule &operator[](size_t i) const {
        return rules_[i];
    }
};

/**
 * @brief Alias for a map of FIRST sets, maps a token to its FIRST set.
 */
using FirstSets = std::map<Token, std::set<Terminal>>;
/**
 * @brief Alias for a map of FOLLOW sets, maps a non-terminal to its FOLLOW set.
 */
using FollowSets = std::unordered_map<NonTerminal, std::set<Terminal>>;

/**
 * @enum ActionType
 * @brief Enum for the type of action in a generated action table.
 */
enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

/**
 * @struct Action
 * @brief Represents an action in the action table.
 */
struct Action {
    /**
     * @brief Stores a type of action.
     */
    ActionType type_ = ActionType::ERROR;
    /**
     * @brief Stores the value related to the action.
     * @details For SHIFT actions, stores the state number to shift to. For
     * REDUCE actions, stores the production number to reduce with. Stores 0 for
     * other ones.
     */
    size_t value_ = 0;
};

/**
 * @brief Alias for an action table, maps a state number and a qualified name of
 * a terminal to an action.
 */
using ActionTable = std::vector<std::unordered_map<std::string, Action>>;
/**
 * @brief Alias for a goto table, maps a state number and a non-terminal to a
 * state number.
 * @details Unlike the action table, the goto table is a map of maps, as goto
 * tables tend to be much sparser than action tables.
 */
using GotoTable =
    std::unordered_map<size_t, std::unordered_map<NonTerminal, size_t>>;
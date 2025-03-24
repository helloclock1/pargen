/**
 * @file Automaton.h
 * @brief Provides a class for interacting with the automaton based on the
 * formal grammar.
 * @author Vadim Melnikov
 * @version 1.0
 */
#pragma once

#include <boost/bimap.hpp>
#include <cstddef>
#include <optional>
#include <set>

#include "Entities.h"
#include "GrammarAnalyzer.h"

/**
 * @class Automaton
 * @brief Represents an automaton, built by the production of the provided
 * formal grammar.
 * @details To be specific, the automaton is a deterministic pushdown one
 * (DPDA). Its nodes, states, are sets of items. Each item is a partially parsed
 * rule and a lookahead token. Transitions are computed based on the next token.
 */
class Automaton {
public:
    /**
     * @struct Item
     * @brief Represents a single item in the automaton state.
     */
    struct Item {
        /**
         * @brief Constructs an Item object.
         * @param rule_number The number of the rule in the grammar.
         * @param dot_pos The position of the dot (pointing at the next unparsed
         * token) in the rule.
         * @param lookahead The lookahead token.
         */
        Item(size_t rule_number, size_t dot_pos, Terminal lookahead);

        /**
         * @brief The number of the rule in the grammar.
         */
        size_t rule_number_;
        /**
         * @brief The position of the dot (pointing at the next unparsed token)
         * in the rule.
         */
        size_t dot_pos_;
        /**
         * @brief The lookahead token.
         */
        Terminal lookahead_;

        friend bool operator<(const Item &lhs, const Item &rhs) {
            return std::tie(lhs.rule_number_, lhs.dot_pos_, lhs.lookahead_) <
                   std::tie(rhs.rule_number_, rhs.dot_pos_, rhs.lookahead_);
        }
    };

    /**
     * @brief An alias for a set of items, representing a state of the
     * automaton.
     */
    using State = std::set<Item>;

    /**
     * @brief An alias for a bidirectional map of states and their numbers.
     */
    using StateMap = boost::bimap<size_t, State>;

    /**
     * @brief Constructs an Automaton object from Grammar and GrammarAnalyzer.
     * @param g The grammar.
     * @param ga The grammar analyzer.
     */
    Automaton(const Grammar &g, const GrammarAnalyzer &ga);

    /**
     * @brief Computes a closure of an arbitrary set of items.
     * @param items The set of items to compute the closure of.
     * @return The closure of the given set of items.
     * @note The closure doesn't have to be a state of the automaton.
     */
    std::set<Item> Closure(const std::set<Item> &items) const;
    /**
     * @brief Computes the next state of the automaton based on the current
     * state and the next token.
     * @param state The state to go to from.
     * @param next The next token.
     * @return The next state of the automaton.
     */
    State Goto(const State &state, const Token &next) const;

    /**
     * @brief Returns the states of the automaton.
     * @return The states of the automaton.
     */
    const StateMap &GetStates() const;

    /**
     * @brief Returns the next token (if exists).
     * @param item The item to get the next token from.
     * @return `std::nullopt` if the dot is at the end of the item, the next
     * token otherwise.
     */
    std::optional<Token> NextToken(const Item &item) const;

private:
    /**
     * @brief Helper function to determine whether the dot is at the end of the
     * given item.
     * @param item The item to check the dot position for.
     * @return `true` if the dot is at the end of the item, `false` otherwise.
     */
    bool DotAtEnd(const Item &item) const;

    /**
     * @brief Computes the canonical collection (all possible states of the
     * automaton) for the grammar.
     */
    void BuildCanonicalCollection();

    const Grammar &g_;
    GrammarAnalyzer ga_;

    StateMap states_;
};
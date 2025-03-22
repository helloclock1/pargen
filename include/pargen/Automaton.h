#pragma once

#include <boost/bimap.hpp>
#include <cstddef>
#include <optional>
#include <set>

#include "Entities.h"
#include "GrammarAnalyzer.h"

class Automaton {
public:
    struct Item {
        Item(size_t rule_number, size_t dot_pos, Terminal lookahead);

        size_t rule_number_;
        size_t dot_pos_;
        Terminal lookahead_;

        friend bool operator<(const Item &lhs, const Item &rhs) {
            return std::tie(lhs.rule_number_, lhs.dot_pos_, lhs.lookahead_) <
                   std::tie(rhs.rule_number_, rhs.dot_pos_, rhs.lookahead_);
        }
    };

    using State = std::set<Item>;

    using StateMap = boost::bimap<size_t, State>;

    Automaton(const Grammar &g, const GrammarAnalyzer &ga);

    State Closure(const State &items) const;
    State Goto(const State &state, Token next) const;

    const StateMap &GetStates() const;

    std::optional<Token> NextToken(const Item &item) const;

private:
    bool DotAtEnd(const Item &item) const;

    void BuildCanonicalCollection();

    const Grammar &g_;
    GrammarAnalyzer ga_;

    StateMap states_;
};
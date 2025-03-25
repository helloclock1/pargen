#include "Automaton.h"

#include "GrammarAnalyzer.h"
#include "Helpers.h"

Automaton::Item::Item(size_t rule_number, size_t dot_pos, Terminal lookahead)
    : rule_number_(rule_number), dot_pos_(dot_pos), lookahead_(lookahead) {
}

Automaton::Automaton(const Grammar &g, const GrammarAnalyzer &ga)
    : g_(g), ga_(ga) {
    BuildCanonicalCollection();
}

std::set<Automaton::Item> Automaton::Closure(
    const std::set<Automaton::Item> &items
) const {
    std::set<Item> closure = items;
    bool changed = true;
    while (changed) {
        changed = false;
        std::set<Item> new_items;
        for (const Item &item : closure) {
            if (DotAtEnd(item)) {
                continue;
            }
            Production p = g_[item.rule_number_].prod;
            Token next_token = p[item.dot_pos_];
            if (IsNonTerminal(next_token)) {
                NonTerminal nt = std::get<NonTerminal>(next_token);
                for (size_t i = 0; i < g_.rules_.size(); ++i) {
                    if (g_[i].lhs == nt) {
                        std::vector<Token> first_seq(
                            p.begin() + item.dot_pos_ + 1, p.end()
                        );
                        if (first_seq.empty()) {
                            first_seq = {EPSILON};
                        }
                        first_seq.push_back(item.lookahead_);
                        std::set<Terminal> result =
                            ga_.FirstForSequence(first_seq);
                        for (const Terminal &t : result) {
                            new_items.insert(Item{i, 0, t});
                        }
                    }
                }
            }
        }
        size_t prev = closure.size();
        closure.insert(new_items.begin(), new_items.end());
        if (closure.size() != prev) {
            changed = true;
        }
    }
    return closure;
}

Automaton::State Automaton::Goto(
    const Automaton::State &state, const Token &next
) const {
    Automaton::State new_state;
    for (const Automaton::Item &item : state) {
        std::optional<Token> next_token = NextToken(item);
        if (next_token.has_value() && next_token.value() == next) {
            new_state.insert(
                Item{item.rule_number_, item.dot_pos_ + 1, item.lookahead_}
            );
        }
    }
    return Closure(new_state);
}

void Automaton::BuildCanonicalCollection() {
    State initial_state = Closure({Item{0, 0, T_EOF}});
    states_.insert({0, initial_state});
    std::set<State> c_set = {initial_state};
    bool changed = true;
    size_t state_idx = 1;
    while (changed) {
        changed = false;
        std::set<State> new_states;
        for (const auto &[idx, state] : states_) {
            for (const Token &token : g_.tokens_) {
                State goto_token_state = Goto(state, token);
                if (!goto_token_state.empty()) {
                    new_states.insert(goto_token_state);
                }
            }
        }
        size_t prev = states_.size();
        for (const State &state : new_states) {
            if (states_.right.find(state) == states_.right.end()) {
                states_.insert({state_idx, state});
                ++state_idx;
            }
        }
        if (states_.size() != prev) {
            changed = true;
        }
        c_set.insert(new_states.begin(), new_states.end());
    }
}

const Automaton::StateMap &Automaton::GetStates() const {
    return states_;
}

bool Automaton::DotAtEnd(const Item &item) const {
    return item.dot_pos_ >= g_[item.rule_number_].prod.size();
}

std::optional<Token> Automaton::NextToken(const Item &item) const {
    if (item.dot_pos_ >= g_[item.rule_number_].prod.size()) {
        return std::nullopt;
    }
    return g_[item.rule_number_].prod[item.dot_pos_];
}
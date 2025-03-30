#include "Automaton.h"

#include <boost/container_hash/hash_fwd.hpp>
#include <cstddef>
#include <queue>

#include "GrammarAnalyzer.h"
#include "Helpers.h"

Automaton::Item::Item(size_t rule_number, size_t dot_pos, Terminal lookahead)
    : rule_number_(rule_number), dot_pos_(dot_pos), lookahead_(lookahead) {
}

Automaton::Automaton(const Grammar &g, const GrammarAnalyzer &ga)
    : g_(g), ga_(ga) {
    BuildCanonicalCollection();
}

bool Automaton::Item::operator==(const Item &other) const {
    return std::tie(rule_number_, dot_pos_, lookahead_) ==
           std::tie(other.rule_number_, other.dot_pos_, other.lookahead_);
}

bool Automaton::ItemSetKey::operator==(const ItemSetKey &other) const {
    return items_ == other.items_;
}

size_t Automaton::ItemSetKeyHash::operator()(const ItemSetKey &key) const {
    size_t seed = 0;
    for (const Item &item : key.items_) {
        boost::hash_combine(seed, std::hash<size_t>()(item.rule_number_));
        boost::hash_combine(seed, std::hash<size_t>()(item.dot_pos_));
        boost::hash_combine(seed, std::hash<Terminal>()(item.lookahead_));
    }
    return seed;
}

std::set<Automaton::Item> Automaton::Closure(
    const std::set<Automaton::Item> &items
) {
    ItemSetKey key = GetKey(items);
    auto it = closure_cache_.find(key);
    if (it != closure_cache_.end()) {
        return it->second;
    }

    std::set<Item> closure = InternalClosure(items);
    closure_cache_[key] = closure;

    return closure;
}

Automaton::State Automaton::Goto(
    const Automaton::State &state, const Token &next
) {
    std::vector<Automaton::Item> new_state_temp;
    for (const Automaton::Item &item : state) {
        std::optional<Token> next_token = NextToken(item);
        if (next_token.has_value() && next_token.value() == next) {
            new_state_temp.push_back(
                Item{item.rule_number_, item.dot_pos_ + 1, item.lookahead_}
            );
        }
    }
    State new_state(new_state_temp.begin(), new_state_temp.end());
    return Closure(new_state);
}

std::set<Automaton::Item> Automaton::InternalClosure(const std::set<Item> &items
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

void Automaton::BuildCanonicalCollection() {
    State initial_state = Closure({Item{0, 0, T_EOF}});
    states_.insert({0, initial_state});
    std::queue<size_t> state_queue;
    state_queue.push(0);
    size_t state_idx = 1;
    while (!state_queue.empty()) {
        size_t current_idx = state_queue.front();
        state_queue.pop();
        const State &current_state = states_.left.at(current_idx);
        for (const Token &token : g_.tokens_) {
            State goto_token_state = Goto(current_state, token);
            if (!goto_token_state.empty()) {
                if (states_.right.find(goto_token_state) ==
                    states_.right.end()) {
                    states_.insert({state_idx, goto_token_state});
                    state_queue.push(state_idx);
                    ++state_idx;
                }
            }
        }
    }
}

const Automaton::StateMap &Automaton::GetStates() const {
    return states_;
}

bool Automaton::DotAtEnd(const Item &item) const {
    return item.dot_pos_ >= g_[item.rule_number_].prod.size();
}

Automaton::ItemSetKey Automaton::GetKey(const std::set<Automaton::Item> &items
) const {
    ItemSetKey key;
    key.items_.reserve(items.size());
    for (const Item &item : items) {
        key.items_.push_back(item);
    }
    std::sort(key.items_.begin(), key.items_.end());
    return key;
}

std::optional<Token> Automaton::NextToken(const Item &item) const {
    if (item.dot_pos_ >= g_[item.rule_number_].prod.size()) {
        return std::nullopt;
    }
    return g_[item.rule_number_].prod[item.dot_pos_];
}
#include "Automaton.h"

Item::Item(size_t rule_number, size_t dot_pos, const Grammar &grammar)
    : rule_number_(rule_number), dot_pos_(dot_pos), grammar_(grammar) {
}

bool IsTerminal(const Token &token) {
    return std::holds_alternative<Terminal>(token);
}

bool IsNonTerminal(const Token &token) {
    return std::holds_alternative<NonTerminal>(token);
}

ParserGenerator::ParserGenerator(const Grammar &g) : g_(g) {
    RetrieveTokens();
    BuildCanonicalCollection();
    ComputeFirst();
    ComputeFollow();
    BuildActionTable();
    BuildGotoTable();
}

ActionTable ParserGenerator::GetActionTable() const {
    return action_;
}

GotoTable ParserGenerator::GetGotoTable() const {
    return goto_;
}

void ParserGenerator::RetrieveTokens() {
    for (const Rule &rule : g_) {
        tokens_.insert(rule.lhs);
        for (const Token &token : rule.prod) {
            tokens_.insert(token);
        }
    }
    tokens_.insert(Terminal{"$"});
}

void ParserGenerator::BuildCanonicalCollection() {
    states_ = {Closure({Item{0, 0, g_}})};
    state_to_number_[states_[0]] = 0;
    std::set<State> c_set{states_.begin(), states_.end()};
    bool changed = true;
    while (changed) {
        changed = false;
        std::set<State> new_states;
        for (const State &state : states_) {
            for (const Token &token : tokens_) {
                State goto_token_state = Goto(state, token);
                if (!goto_token_state.empty() &&
                    c_set.find(goto_token_state) == c_set.end() &&
                    new_states.find(goto_token_state) == new_states.end()) {
                    new_states.insert(goto_token_state);
                    changed = true;
                }
            }
        }
        for (const State &state : new_states) {
            state_to_number_[state] = states_.size();
            states_.push_back(state);
        }
        c_set.insert(new_states.begin(), new_states.end());
    }
}

void ParserGenerator::BuildActionTable() {
    action_.resize(states_.size());
    // iterate through all items in all states
    for (size_t i = 0; i < states_.size(); ++i) {
        // i --- state number
        for (const Item &item : states_[i]) {
            std::optional<Token> next_token_opt = item.NextToken();
            if (next_token_opt.has_value()) {
                Token next_token = next_token_opt.value();
                if (IsTerminal(next_token)) {
                    size_t next_state_j =
                        state_to_number_[Goto(states_[i], next_token)];
                    action_[i][std::get<Terminal>(next_token)] =
                        Action{ActionType::SHIFT, next_state_j};
                }
            } else {
                if (item.rule_number_ != 0) {
                    NonTerminal lhs = g_[item.rule_number_].lhs;
                    for (const Terminal &t : follow_[lhs]) {
                        if (action_[i].find(t) != action_[i].end()) {
                            throw std::runtime_error("reduce/reduce conflict");
                        }
                        action_[i][t] =
                            Action{ActionType::REDUCE, item.rule_number_};
                    }
                } else {
                    action_[i][Terminal{"$"}] = Action{ActionType::ACCEPT};
                }
            }
        }
    }
    for (size_t i = 0; i < states_.size(); ++i) {
        for (const Token &token : tokens_) {
            if (IsNonTerminal(token)) {
                continue;
            }
            Terminal t = std::get<Terminal>(token);
            Action e = action_[i][t];
            switch (e.type_) {
                case ActionType::SHIFT:
                    break;
                case ActionType::REDUCE:
                    break;
                case ActionType::ACCEPT:
                    break;
                case ActionType::ERROR:
                    break;
            }
        }
    }
}

void ParserGenerator::BuildGotoTable() {
    for (size_t i = 0; i < states_.size(); ++i) {
        for (const Token &token : tokens_) {
            if (IsTerminal(token)) {
                continue;
            }
            NonTerminal nt = std::get<NonTerminal>(token);
            size_t goto_value = state_to_number_[Goto(states_[i], nt)];
            if (goto_value == 0) {
                continue;
            }
            goto_[i][nt] = goto_value;
        }
    }
}

void ParserGenerator::ComputeFirst() {
    for (const Token &token : tokens_) {
        if (IsTerminal(token)) {
            first_[token] = {std::get<Terminal>(token)};
        } else {
            first_[token] = {};
        }
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Rule &rule : g_) {
            bool include_eps = true;
            for (const Token &token : rule.prod) {
                size_t prev_size = first_[rule.lhs].size();
                std::set<Terminal> token_first = first_[token];
                auto eps_location = token_first.find(Terminal{""});
                bool eps_in_token_first = token_first.contains(Terminal{""});
                if (eps_in_token_first) {
                    token_first.erase(eps_location);
                }
                first_[rule.lhs].insert(token_first.begin(), token_first.end());
                if (first_[rule.lhs].size() != prev_size) {
                    changed = true;
                }
                if (!eps_in_token_first) {
                    include_eps = false;
                    break;
                }
            }
            if (include_eps) {
                if (!first_[rule.lhs].contains(Terminal{""})) {
                    changed = true;
                }
                first_[rule.lhs].insert(Terminal{""});
            }
        }
    }
}

std::set<Terminal> ParserGenerator::FirstForSequence(std::vector<Token> seq) {
    if (seq.empty()) {
        return {Terminal{""}};
    }
    std::set<Terminal> result;
    bool eps_in_prev = true;
    size_t i = 0;
    while (eps_in_prev && i < seq.size()) {
        std::set<Terminal> token_first = first_[seq[i]];
        bool eps_in_token = token_first.contains(Terminal{""});
        if (eps_in_token) {
            token_first.erase(Terminal{""});
        }
        result.insert(token_first.begin(), token_first.end());
        eps_in_prev = eps_in_token;
        ++i;
    }
    if (eps_in_prev) {
        result.insert(Terminal{""});
    }
    return result;
}

void ParserGenerator::ComputeFollow() {
    follow_[g_[0].lhs] = {Terminal{"$"}};
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Rule &rule : g_) {
            for (size_t i = 0; i < rule.prod.size(); ++i) {
                if (IsTerminal(rule.prod[i])) {
                    continue;
                }
                NonTerminal token = std::get<NonTerminal>(rule.prod[i]);
                size_t prev_size = follow_[token].size();
                if (i != rule.prod.size() - 1) {
                    std::vector<Token> beta(rule.prod.begin() + i + 1,
                                            rule.prod.end());
                    auto to_add = FirstForSequence(beta);
                    if (to_add.contains(Terminal{""})) {
                        to_add.erase(to_add.find(Terminal{""}));
                        follow_[token].insert(follow_[rule.lhs].begin(),
                                              follow_[rule.lhs].end());
                    }
                    follow_[token].insert(to_add.begin(), to_add.end());
                } else {
                    follow_[token].insert(follow_[rule.lhs].begin(),
                                          follow_[rule.lhs].end());
                }
                if (follow_[token].size() != prev_size) {
                    changed = true;
                }
            }
        }
    }
}

std::set<Item> ParserGenerator::Closure(const std::set<Item> &items) {
    std::set<Item> closure = items;
    while (true) {
        std::set<Item> new_items;
        for (const Item &state : closure) {
            if (state.dot_pos_ >= g_[state.rule_number_].prod.size()) {
                continue;
            }
            Token next_token = g_[state.rule_number_].prod[state.dot_pos_];
            if (IsNonTerminal(next_token)) {
                for (size_t i = 0; i < g_.size(); ++i) {
                    Item to_add{i, 0, g_};
                    if (!closure.contains(to_add) &&
                        !new_items.contains(to_add) &&
                        g_[i].lhs == std::get<NonTerminal>(next_token)) {
                        new_items.insert(to_add);
                    }
                }
            }
        }
        size_t before = closure.size();
        closure.insert(new_items.begin(), new_items.end());
        if (closure.size() == before) {
            break;
        }
    }
    return closure;
}

State ParserGenerator::Goto(State state, Token next) {
    State new_state;
    for (const Item &item : state) {
        std::optional<Token> next_token = item.NextToken();
        if (next_token.has_value() && next_token.value() == next) {
            new_state.insert(
                Item{item.rule_number_, item.dot_pos_ + 1, item.grammar_});
        }
    }
    return Closure(new_state);
}

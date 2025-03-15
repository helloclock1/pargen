#include "Automaton.h"

#include "Entities.h"

Item::Item(size_t rule_number, size_t dot_pos, Terminal lookahead,
           const Grammar &grammar)
    : rule_number_(rule_number),
      dot_pos_(dot_pos),
      lookahead_(lookahead),
      grammar_(grammar) {
}

bool Item::DotAtEnd() const {
    return dot_pos_ >= grammar_[rule_number_].prod.size();
}

std::optional<Token> Item::NextToken() const {
    if (!DotAtEnd()) {
        return grammar_[rule_number_].prod[dot_pos_];
    }
    return std::nullopt;
}

bool IsTerminal(const Token &token) {
    return std::holds_alternative<Terminal>(token);
}

bool IsNonTerminal(const Token &token) {
    return std::holds_alternative<NonTerminal>(token);
}

std::string QualName(Token token) {
    if (IsTerminal(token)) {
        Terminal t = std::get<Terminal>(token);
        if (t.repr_.empty()) {
            return "T_" + t.name_;
        } else {
            return "R_" + t.name_;
        }
    } else {
        return "NT_" + std::get<NonTerminal>(token).name_;
    }
}

ParserGenerator::ParserGenerator(const Grammar &g) : g_(g) {
    g_.tokens_.insert(Terminal{"$"});
    ComputeFirst();
    BuildCanonicalCollection();
    BuildActionTable();
    BuildGotoTable();
}

ActionTable ParserGenerator::GetActionTable() const {
    return action_;
}

GotoTable ParserGenerator::GetGotoTable() const {
    return goto_;
}

void ParserGenerator::BuildCanonicalCollection() {
    states_ = {Closure({Item{0, 0, Terminal{"$"}, g_}})};
    state_to_number_[states_[0]] = 0;
    std::set<State> c_set{states_.begin(), states_.end()};
    bool changed = true;
    while (changed) {
        changed = false;
        std::set<State> new_states;
        for (const State &state : states_) {
            for (const Token &token : g_.tokens_) {
                State goto_token_state = Goto(state, token);
                if (!goto_token_state.empty() &&
                    !c_set.contains(goto_token_state) &&
                    !new_states.contains(goto_token_state)) {
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
    for (size_t i = 0; i < states_.size(); ++i) {
        for (const Item &item : states_[i]) {
            std::optional<Token> next_token_opt = item.NextToken();
            if (next_token_opt.has_value()) {
                Token next_token = next_token_opt.value();
                if (IsTerminal(next_token)) {
                    size_t next_state_j =
                        state_to_number_[Goto(states_[i], next_token)];
                    if (std::get<Terminal>(next_token) != EPSILON) {
                        action_[i][QualName(std::get<Terminal>(next_token))] =
                            Action{ActionType::SHIFT, next_state_j};
                    } else {  // this should be at the end
                        action_[i][QualName(item.lookahead_)] =
                            Action{ActionType::REDUCE, item.rule_number_};
                    }
                }
            } else {
                if (item.rule_number_ != 0) {
                    NonTerminal lhs = g_[item.rule_number_].lhs;
                    action_[i][QualName(item.lookahead_)] =
                        Action{ActionType::REDUCE, item.rule_number_};
                } else {
                    action_[i][QualName(Terminal{"$"})] =
                        Action{ActionType::ACCEPT};
                }
            }
        }
    }
}

void ParserGenerator::BuildGotoTable() {
    for (size_t i = 0; i < states_.size(); ++i) {
        for (const Token &token : g_.tokens_) {
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
    for (const Token &token : g_.tokens_) {
        if (IsTerminal(token)) {
            first_[token] = {std::get<Terminal>(token)};
        } else {
            first_[token] = {};
        }
    }
    first_[EPSILON] = {EPSILON};
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Rule &rule : g_.rules_) {
            bool include_eps = true;
            for (const Token &token : rule.prod) {
                size_t prev_size = first_[rule.lhs].size();
                std::set<Terminal> token_first = first_[token];
                auto eps_location = token_first.find(EPSILON);
                bool eps_in_token_first = token_first.contains(EPSILON);
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
                if (!first_[rule.lhs].contains(EPSILON)) {
                    changed = true;
                }
                first_[rule.lhs].insert(EPSILON);
            }
        }
    }
}

std::set<Terminal> ParserGenerator::FirstForSequence(
    const std::vector<Token> &seq) {
    if (seq.empty()) {
        return {EPSILON};
    }
    std::set<Terminal> result;
    bool eps_in_prev = true;
    size_t i = 0;
    while (eps_in_prev && i < seq.size()) {
        std::set<Terminal> token_first = first_[seq[i]];
        bool eps_in_token = token_first.contains(EPSILON);
        if (eps_in_token) {
            token_first.erase(EPSILON);
        }
        result.insert(token_first.begin(), token_first.end());
        eps_in_prev = eps_in_token;
        ++i;
    }
    if (eps_in_prev) {
        result.insert(EPSILON);
    }
    return result;
}

std::set<Item> ParserGenerator::Closure(const std::set<Item> &items) {
    std::set<Item> closure = items;
    bool changed = true;
    while (changed) {
        changed = false;
        std::set<Item> new_items;
        for (const Item &item : closure) {
            if (item.DotAtEnd()) {
                continue;
            }
            Production p = g_[item.rule_number_].prod;
            Token next_token = p[item.dot_pos_];
            if (IsNonTerminal(next_token)) {
                NonTerminal nt = std::get<NonTerminal>(next_token);
                for (size_t i = 0; i < g_.rules_.size(); ++i) {
                    if (g_[i].lhs == nt) {
                        std::vector<Token> first_seq(
                            p.begin() + item.dot_pos_ + 1, p.end());
                        if (first_seq.empty()) {
                            first_seq = {EPSILON};
                        }
                        first_seq.push_back(item.lookahead_);
                        std::set<Terminal> result = FirstForSequence(first_seq);
                        for (const Terminal &t : result) {
                            new_items.insert(Item{i, 0, t, g_});
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

State ParserGenerator::Goto(State state, Token next) {
    State new_state;
    for (const Item &item : state) {
        std::optional<Token> next_token = item.NextToken();
        if (next_token.has_value() && next_token.value() == next) {
            new_state.insert(Item{item.rule_number_, item.dot_pos_ + 1,
                                  item.lookahead_, item.grammar_});
        }
    }
    return Closure(new_state);
}

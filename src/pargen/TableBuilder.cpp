#include "TableBuilder.h"

#include "Entities.h"
#include "GrammarAnalyzer.h"
#include "Helpers.h"

TableGeneratorError::TableGeneratorError(const std::string &msg) : msg_(msg) {
}

const char *TableGeneratorError::what() const noexcept {
    return msg_.c_str();
}

ParserTables::ParserTables(const Grammar &g, const GrammarAnalyzer &ga)
    : g_(g), automaton_(g, ga), states_(automaton_.GetStates()) {
}

void ParserTables::Generate() {
    BuildActionTable();
    BuildGotoTable();
}

ActionTable ParserTables::GetActionTable() const {
    return action_;
}

GotoTable ParserTables::GetGotoTable() const {
    return goto_;
}

void ParserTables::BuildActionTable() {
    action_.resize(states_.size());
    for (size_t i = 0; i < states_.size(); ++i) {
        for (const Automaton::Item &item : states_.left.at(i)) {
            std::optional<Token> next_token_opt = automaton_.NextToken(item);
            if (next_token_opt.has_value()) {
                Token next_token = next_token_opt.value();
                if (IsTerminal(next_token)) {
                    Automaton::State next_state =
                        automaton_.Goto(states_.left.at(i), next_token);
                    size_t next_state_j = 0;
                    if (states_.right.find(next_state) != states_.right.end()) {
                        next_state_j = states_.right.at(next_state);
                    }
                    if (std::get<Terminal>(next_token) != EPSILON) {
                        std::string key =
                            QualName(std::get<Terminal>(next_token));
                        Action new_action{ActionType::SHIFT, next_state_j};
                        if (action_[i].find(key) != action_[i].end()) {
                            Action existing = action_[i][key];
                            if (existing.type_ == ActionType::REDUCE) {
                                throw TableGeneratorError(
                                    "Provided grammar is ambiguous "
                                    "(shift/reduce conflict on token: " +
                                    key + ")"
                                );
                            }
                            if (existing.type_ == ActionType::SHIFT &&
                                existing.value_ != new_action.value_) {
                                throw TableGeneratorError(
                                    "Provided grammar is ambiguous "
                                    "(shift/shift conflict on token: " +
                                    key + ")"
                                );
                            }
                        }
                        action_[i][key] = new_action;
                    } else {
                        std::string key = QualName(item.lookahead_);
                        Action new_action{
                            ActionType::REDUCE, item.rule_number_
                        };
                        if (action_[i].find(key) != action_[i].end()) {
                            Action existing = action_[i][key];
                            if (existing.type_ == ActionType::SHIFT) {
                                throw TableGeneratorError(
                                    "Provided grammar is ambiguous "
                                    "(shift/reduce conflict on token: " +
                                    key + ")"
                                );
                            }
                            if (existing.type_ == ActionType::REDUCE &&
                                existing.value_ != new_action.value_) {
                                throw TableGeneratorError(
                                    "Provided grammar is ambiguous "
                                    "(reduce/reduce conflict on token: " +
                                    key + ")"
                                );
                            }
                        }
                        action_[i][key] = new_action;
                    }
                }
            } else {
                std::string key;
                Action new_action;
                if (item.rule_number_ != 0) {
                    key = QualName(item.lookahead_);
                    new_action = Action{ActionType::REDUCE, item.rule_number_};
                } else {
                    key = QualName(T_EOF);
                    new_action = Action{ActionType::ACCEPT};
                }
                if (action_[i].find(key) != action_[i].end()) {
                    Action existing = action_[i][key];
                    if (existing.type_ == ActionType::SHIFT ||
                        (existing.type_ == ActionType::REDUCE &&
                         existing.value_ != new_action.value_)) {
                        throw TableGeneratorError(
                            "Provided grammar is ambiguous (conflict in action "
                            "table on token: " +
                            key + ")"
                        );
                    }
                }
                action_[i][key] = new_action;
            }
        }
    }
}

void ParserTables::BuildGotoTable() {
    for (size_t i = 0; i < states_.size(); ++i) {
        for (const Token &token : g_.tokens_) {
            if (IsTerminal(token)) {
                continue;
            }
            NonTerminal nt = std::get<NonTerminal>(token);
            Automaton::State state = automaton_.Goto(states_.left.at(i), nt);
            if (states_.right.find(state) != states_.right.end()) {
                goto_[i][nt] = states_.right.at(state);
            }
        }
    }
}
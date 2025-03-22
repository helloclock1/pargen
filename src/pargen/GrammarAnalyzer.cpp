#include "GrammarAnalyzer.h"

#include "Entities.h"
#include "Helpers.h"

GrammarAnalyzer::GrammarAnalyzer(const Grammar &g) : g_(g) {
    ComputeFirst();
    ComputeFollow();
}

void GrammarAnalyzer::ComputeFirst() {
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

std::set<Terminal> GrammarAnalyzer::FirstForSequence(
    const std::vector<Token> &seq
) const {
    if (seq.empty()) {
        return {EPSILON};
    }
    std::set<Terminal> result;
    bool eps_in_prev = true;
    size_t i = 0;
    while (eps_in_prev && i < seq.size()) {
        std::set<Terminal> token_first;  // = first_.at(seq[i]);
        if (first_.contains(seq[i])) {
            token_first = first_.at(seq[i]);
        } else {
            token_first = {};
        }
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

void GrammarAnalyzer::ComputeFollow() {
    follow_[g_[0].lhs] = {T_EOF};
    bool changed = true;
    while (changed) {
        changed = false;
        for (const Rule &rule : g_.rules_) {
            for (size_t i = 0; i < rule.prod.size(); ++i) {
                if (IsTerminal(rule.prod[i])) {
                    continue;
                }
                NonTerminal token = std::get<NonTerminal>(rule.prod[i]);
                size_t prev_size = follow_[token].size();
                if (i != rule.prod.size() - 1) {
                    std::vector<Token> beta(
                        rule.prod.begin() + i + 1, rule.prod.end()
                    );
                    auto to_add = FirstForSequence(beta);
                    if (to_add.contains(EPSILON)) {
                        to_add.erase(to_add.find(EPSILON));
                        follow_[token].insert(
                            follow_[rule.lhs].begin(), follow_[rule.lhs].end()
                        );
                    }
                    follow_[token].insert(to_add.begin(), to_add.end());
                } else {
                    follow_[token].insert(
                        follow_[rule.lhs].begin(), follow_[rule.lhs].end()
                    );
                }
                if (follow_[token].size() != prev_size) {
                    changed = true;
                }
            }
        }
    }
}

const FirstSets &GrammarAnalyzer::GetFirst() const {
    return first_;
}

const FollowSets &GrammarAnalyzer::GetFollow() const {
    return follow_;
}
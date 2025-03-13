#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <unordered_map>

#include "Entities.h"

struct Item {
    Item(size_t rule_number, size_t dot_pos, const Grammar &grammar);

    size_t rule_number_;
    size_t dot_pos_;
    const Grammar &grammar_;
    // dot_pos_ = i means that dot is placed before i-th token,
    // next token is on position i respectively

    friend bool operator<(const Item &a, const Item &b) {
        return a.rule_number_ < b.rule_number_ ||
               (a.rule_number_ == b.rule_number_ && a.dot_pos_ < b.dot_pos_);
    }

    bool DotAtEnd() const {
        bool a = dot_pos_ >= grammar_[rule_number_].prod.size();
        return a;
    }

    std::optional<Token> NextToken() const {
        if (!DotAtEnd()) {
            return grammar_[rule_number_].prod[dot_pos_];
        }
        return std::nullopt;
    }

    Rule GetRule() const {
        Rule rule;
        rule.lhs = grammar_[rule_number_].lhs;
        for (size_t i = 0; i < grammar_[rule_number_].prod.size(); ++i) {
            if (i == dot_pos_) {
                rule.prod.push_back(NonTerminal{"."});
            }
            rule.prod.push_back(grammar_[rule_number_].prod[i]);
        }
        return rule;
    }
};

using State = std::set<Item>;

class AutomatonNode {
    State state_;
    std::unordered_map<Terminal, std::unique_ptr<AutomatonNode>> to_;
};

enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

struct Action {
    ActionType type_ = ActionType::ERROR;
    size_t value_ = 0;
};

// TODO(helloclock): maybe rewrite to umap + hash function
using ActionTable = std::vector<std::unordered_map<std::string, Action>>;
using GotoTable =
    std::unordered_map<size_t, std::unordered_map<NonTerminal, size_t>>;

std::string QualName(Token token);

class ParserGenerator {
public:
    ParserGenerator() = delete;
    ParserGenerator(const Grammar &g);

    ActionTable GetActionTable() const;
    GotoTable GetGotoTable() const;
    std::vector<State> GetStates() const {
        return states_;
    }
    std::map<State, size_t> GetStateToNumber() const {
        return state_to_number_;
    }

private:
    // TODO(helloclock): too much stuff here, rewrite/split
    void RetrieveTokens();
    void BuildCanonicalCollection();
    void BuildActionTable();
    void BuildGotoTable();

    std::map<Token, std::set<Terminal>> first_;
    void ComputeFirst();
    std::set<Terminal> FirstForSequence(std::vector<Token> seq);

    std::unordered_map<NonTerminal, std::set<Terminal>> follow_;
    void ComputeFollow();

    std::set<Item> Closure(const std::set<Item> &items);
    State Goto(State state, Token next);

    Grammar g_;
    std::set<Token> tokens_;

    std::map<State, size_t> state_to_number_;
    std::vector<State> states_;

    AutomatonNode *current_state_;
    ActionTable action_;
    GotoTable goto_;
};
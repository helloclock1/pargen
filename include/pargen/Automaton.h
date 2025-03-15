#pragma once

#include <boost/bimap.hpp>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <unordered_map>

#include "Entities.h"

// TODO(helloclock): move those to a separate header alongside other helper
// functions
const Terminal EPSILON = Terminal{""};

bool IsTerminal(const Token &token);

bool IsNonTerminal(const Token &token);

struct Item {
    Item(size_t rule_number, size_t dot_pos, Terminal lookahead,
         const Grammar &grammar);

    size_t rule_number_;
    size_t dot_pos_;
    Terminal lookahead_;
    // TODO(helloclock): get rid of this field, move Item decl inside
    // ParserGenerator
    const Grammar &grammar_;
    // dot_pos_ = i means that dot is placed before i-th token,
    // next token is on position i respectively

    friend bool operator<(const Item &a, const Item &b) {
        return std::tie(a.rule_number_, a.dot_pos_, a.lookahead_) <
               std::tie(b.rule_number_, b.dot_pos_, b.lookahead_);
    }

    bool DotAtEnd() const;

    std::optional<Token> NextToken() const;
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

private:
    // TODO(helloclock): too much stuff here, rewrite/split
    void BuildCanonicalCollection();
    void BuildActionTable();
    void BuildGotoTable();

    std::map<Token, std::set<Terminal>> first_;
    void ComputeFirst();
    std::set<Terminal> FirstForSequence(const std::vector<Token> &seq);

    std::set<Item> Closure(const std::set<Item> &items);
    State Goto(State state, Token next);

    Grammar g_;

    boost::bimap<size_t, State> states_;

    AutomatonNode *current_state_;
    ActionTable action_;
    GotoTable goto_;
};
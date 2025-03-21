#pragma once

#include <boost/bimap.hpp>
#include <boost/container_hash/hash.hpp>
#include <map>
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

enum class ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

struct Action {
    ActionType type_ = ActionType::ERROR;
    size_t value_ = 0;
};

using ActionTable = std::vector<std::unordered_map<std::string, Action>>;
using GotoTable =
    std::unordered_map<size_t, std::unordered_map<NonTerminal, size_t>>;

std::string QualName(Token token);

class ParserGeneratorError : public std::exception {
public:
    explicit ParserGeneratorError(const std::string &msg);
    const char *what() const noexcept override;

private:
    std::string msg_;
};

class ParserGenerator {
public:
    ParserGenerator() = delete;
    ParserGenerator(const Grammar &g);

    void Generate();

    ActionTable GetActionTable() const;
    GotoTable GetGotoTable() const;

    struct Item {
        Item(size_t rule_number, size_t dot_pos, Terminal lookahead);

        size_t rule_number_;
        size_t dot_pos_;
        Terminal lookahead_;

        friend bool operator<(const Item &a, const Item &b) {
            return std::tie(a.rule_number_, a.dot_pos_, a.lookahead_) <
                   std::tie(b.rule_number_, b.dot_pos_, b.lookahead_);
        }
    };

    using State = std::set<Item>;

private:
    bool DotAtEnd(const Item &item) const;

    std::optional<Token> NextToken(const Item &item) const;

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

    ActionTable action_;
    GotoTable goto_;
};
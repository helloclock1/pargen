#pragma once

#include "Automaton.h"
#include "Entities.h"
#include "GrammarAnalyzer.h"

class TableGeneratorError : public std::exception {
public:
    explicit TableGeneratorError(const std::string &msg);
    const char *what() const noexcept override;

private:
    std::string msg_;
};

class ParserTables {
public:
    ParserTables(const Grammar &g, const GrammarAnalyzer &ga);

    void Generate();

    ActionTable GetActionTable() const;
    GotoTable GetGotoTable() const;

private:
    void BuildActionTable();
    void BuildGotoTable();

    Automaton automaton_;
    Automaton::StateMap states_;
    const Grammar &g_;

    ActionTable action_;
    GotoTable goto_;
};
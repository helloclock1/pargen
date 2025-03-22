/**
 * @file TableBuilder.h
 * @brief Provides a class for generating parser tables.
 * @author Vadim Melnikov
 * @version 1.0
 */
#pragma once

#include "Automaton.h"
#include "Entities.h"
#include "GrammarAnalyzer.h"

/**
 * @class TableGeneratorError
 * @brief An exception class for reporting errors in the process of table
 * generation.
 */
class TableGeneratorError : public std::exception {
public:
    /**
     * @brief Constructs a TableGeneratorError object with the specified error.
     * @param msg The error message.
     */
    explicit TableGeneratorError(const std::string &msg);
    /**
     * @brief Returns the error message.
     * @return The error message.
     */
    const char *what() const noexcept override;

private:
    std::string msg_;
};

/**
 * @class ParserTables
 * @brief A class for generating parser tables.
 */
class ParserTables {
public:
    /**
     * @brief Constructs a ParserTables object with the specified grammar and
     * specified GrammarAnalyzer.
     * @param g The grammar to generate tables for.
     * @param ga The GrammarAnalyzer that provides pregenerated sets for the
     * grammar.
     */
    ParserTables(const Grammar &g, const GrammarAnalyzer &ga);

    /**
     * @brief Generates both parser tables.
     */
    void Generate();

    /**
     * @brief Returns the action table.
     */
    ActionTable GetActionTable() const;
    /**
     * @brief Returns the goto table.
     */
    GotoTable GetGotoTable() const;

private:
    /**
     * @brief Builds the action table.
     * @throws TableGeneratorError if the provided grammar is ambiguous
     * (equally, if there is a shift/shift or shift/reduce or reduce/reduce in
     * the process of building an action table).
     */
    void BuildActionTable();
    /**
     * @brief Builds the goto table.
     */
    void BuildGotoTable();

    Automaton automaton_;
    Automaton::StateMap states_;
    const Grammar &g_;

    ActionTable action_;
    GotoTable goto_;
};
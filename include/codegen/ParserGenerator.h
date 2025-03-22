/**
 * @file ParserGenerator.h
 * @brief Provides a class for generating a parser from a given grammar, objects
 * related to the parser and a few flags.
 * @author Vadim Melnikov
 * @version 1.0
 */
#pragma once

#include "Entities.h"

/**
 * @class ParserGeneratorError
 * @brief Exception class for reporting errors in the process of generating a
 * parser.
 */
class ParserGeneratorError : public std::exception {
public:
    /**
     * @brief Constructs a ParserGeneratorError object with the specified error.
     */
    explicit ParserGeneratorError(const std::string &msg);

    /**
     * @brief Returns the error message.
     * @return The error message.
     */
    const char *what() const noexcept override;

private:
    std::string msg_;
};

/**
 * @class ParserGenerator
 * @brief Generates a parser for the given grammar to a folder.
 */
class ParserGenerator {
public:
    /**
     * @brief Constructs a ParserGenerator object with the specified grammar.
     * @param folder The folder to generate the parser to.
     * @param g The grammar to generate the parser for.
     * @param at The action table to use for the generated parser.
     * @param gt The goto table to use for the generated parser.
     * @param fs The FOLLOW sets to use for the generated parser.
     * @param add_json_generator Whether to add a JSON parse tree generator to
     * the generated parser.
     * @param json_indents The number of indents to use for the JSON parse tree
     * (if it is generated).
     */
    ParserGenerator(
        const std::string &folder, const Grammar &g, const ActionTable &at,
        const GotoTable &gt, const FollowSets &fs, bool add_json_generator,
        size_t json_indents
    );

    /**
     * @brief Generates the parser.
     */
    void Generate();

private:
    std::string folder_;
    const Grammar &g_;
    const ActionTable &at_;
    const GotoTable &gt_;
    const FollowSets &fs_;
    bool add_json_generator_;
    size_t json_indents_;
};
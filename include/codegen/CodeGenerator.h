/**
 * @file CodeGenerator.h
 * @brief Provides a class for generating all code for the parser.
 * @author Vadim Melnikov
 * @version 1.0
 */
#include <cstdlib>
#include <cstring>

#include "Entities.h"

/**
 * @class CodeGeneratorError
 * @brief Exception class for reporting errors in the process of generating code
 * for the parser.
 */
class CodeGeneratorError : public std::exception {
public:
    /**
     * @brief Constructs a CodeGeneratorError object with the specified error.
     */
    explicit CodeGeneratorError(const std::string &msg);

    /**
     * @brief Returns the error message.
     * @return The error message.
     */
    const char *what() const noexcept override;

private:
    std::string msg_;
};

/**
 * @class CodeGenerator
 * @brief Generates all code for the parser.
 */
class CodeGenerator {
public:
    /**
     * @brief Constructs a CodeGenerator object with the specified parameters.
     * @param folder The folder to generate the code to.
     * @param at The action table to use for the generated code
     * @param gt The goto table to use for the generated code.
     * @param fs The FOLLOW sets to use for the generated code.
     * @param g The grammar to generate the code for.
     * @param add_json_generator Whether to add a JSON parse tree generator to
     * the generated parser.
     * @param json_indents The number of indents to use for the JSON parse tree
     * (if it is generated).
     */
    CodeGenerator(
        const std::string &folder, ActionTable &at, GotoTable &gt,
        FollowSets &fs, const Grammar &g, bool add_json_generator,
        size_t json_indents
    );

    /**
     * @brief Generates all code for the parser.
     * @throws CodeGeneratorError if an error occurs while generating the code,
     * but the error is not related to either a lexer or a parser generator.
     * Rethrows LexerGeneratorError or ParserGeneratorError if they are thrown.
     */
    void Generate();

private:
    std::string folder_;
    Grammar g_;
    ActionTable &at_;
    GotoTable &gt_;
    FollowSets &fs_;
    bool add_json_generator_;
    size_t json_indents_;
};
/**
 * @file LexerGenerator.h
 * @brief Provides a class for generating a lexer based on the provided grammar.
 * @author Vadim Melnikov
 * @version 1.0
 */
#pragma once

#include <string>

#include "Entities.h"

/**
 * @class LexerGeneratorError
 * @brief An exception class for reporting errors in the process of generating a
 * lexer.
 */
class LexerGeneratorError : public std::exception {
public:
    /**
     * @brief Constructs a LexerGeneratorError object with the specified error.
     * @param msg The error message.
     */
    explicit LexerGeneratorError(const std::string &msg);

    /**
     * @brief Returns the error message.
     * @return The error message.
     */
    const char *what() const noexcept override;

private:
    std::string msg_;
};

/**
 * @class LexerGenerator
 * @brief A class for generating a lexer based on the provided grammar.
 */
class LexerGenerator {
public:
    /**
     * @brief Constructs a LexerGenerator object.
     * @param folder A folder that the lexer is generated to.
     * @param g The grammar to generated the lexer for.
     */
    LexerGenerator(const std::string &folder, const Grammar &g);

    /**
     * @brief Generates the lexer.
     * @note The lexer is generated using `flex` utility that often comes
     * preinstalled with UNIX-based systems.
     */
    void Generate();

private:
    std::string folder_;
    const Grammar &g_;
};
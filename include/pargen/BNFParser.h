/**
 * @file BNFParser.h
 * @brief Provides a class for parsing a user-specified grammar in BNF.
 * @author Vadim Melnikov
 * @version 1.0
 */
#pragma once

#include <cctype>
#include <cstring>
#include <iostream>
#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "Entities.h"

/**
 * @class GrammarParserError
 * @brief An exception class for reporting errors in the process of parsing a
 * grammar.
 */
class GrammarParserError : public std::exception {
public:
    /**
     * @brief Constructs a GrammarParserError object with the specified error
     * and line number.
     * @param msg The error message.
     * @param line The line number where the error occurred.
     */
    explicit GrammarParserError(const std::string &msg, size_t line);

    /**
     * @brief Returns the error message.
     * @return The error message.
     */
    const char *what() const noexcept override;

private:
    static std::string ConstructMessage(const std::string &msg, size_t line);
    std::string msg_;
};

/**
 * @class GrammarParser
 * @brief A class for parsing a user-specified grammar in BNF.
 * @details This class provides a mechanism for parsing a user-specified grammar
 * in BNF. It reads the grammar from an input stream and stores the parsed
 * grammar in a Grammar object.
 */
class GrammarParser {
public:
    /**
     * @brief Constructs a GrammarParser object with the specified input stream.
     * @param in A unique pointer to the input stream that the grammar is read
     * from.
     */
    GrammarParser(std::unique_ptr<std::istream> in);

    /**
     * @brief Parses the grammar from the input stream.
     * @throws GrammarParserError if an error occurs during parsing
     * (GrammarParserError can be thrown by the functions called by this
     * function too).
     */
    void Parse();

    /**
     * @brief Returns the parsed grammar.
     * @return Const reference to the parsed grammar.
     */
    const Grammar &Get() const;

private:
    /**
     * @brief Helper function for reading a character from stream.
     * @return The character read from the stream (or EOF if `in_` is peeking at
     * EOF).
     */
    int GetChar();
    /**
     * @brief Helper function for reading a character from stream.
     * @param expected The character that is expected to be read.
     * @return The character read from the stream (or EOF if `in_` is peeking at
     * EOF).
     * @throws GrammarParserError if the character read is not equal to
     * `expected`.
     */
    int GetChar(int expected);
    /**
     * @brief Helper function for peeking a character from stream.
     * @return The character that `in_` is peeking at (or EOF if `in_` is
     * peeking at EOF).
     */
    int Peek() const;
    /**
     * @brief Helper function for comparing a character that `in_` is peeking at
     * against a desired character.
     * @param c The character to compare against.
     * @return true if the character that `in_` is peeking at is equal to `c`,
     * false otherwise.
     */
    bool PeekAt(int c) const;
    /**
     * @brief Helper function for skipping whitespace characters.
     */
    void SkipWS();

    /**
     * @brief Reads a line from the input stream.
     * @throws GrammarParserError if an error occurs during reading the line
     * (e.g., quote terminal on LHS of an expression).
     */
    void ParseLine();
    /**
     * @brief Parses a grammar rule from the input stream.
     * @return The parsed rule.
     * @throws GrammarParserError if an error occurs during parsing the rule
     * (e.g., undefined regex terminal).
     * @todo Change return type to `Production`
     */
    std::vector<Token> ParseRule();
    /**
     * @brief Parses a token from the input stream.
     * @return The parsed token.
     * @throws GrammarParserError if an error occurs during parsing the token
     * (e.g., unterminated token specifier).
     */
    Token ParseToken();
    /**
     * @brief Parses a quote terminal from the input stream.
     * @return The parsed quote terminal.
     * @throws GrammarParserError if an error occurs during parsing the quote
     * terminal.
     */
    Terminal ParseQuoteTerminal();
    /**
     * @brief Parses a qualifier from the input stream.
     * @return The parsed qualifier.
     */
    std::string ParseName();
    /**
     * @brief Parses a regex for the characters to be ignored by the lexer
     * produced in future.
     */
    void ParseIgnore();

    std::unique_ptr<std::istream> in_;
    size_t line_ = 0;
    Grammar g_;
};
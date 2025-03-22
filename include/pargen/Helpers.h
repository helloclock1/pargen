/**
 * @file Helpers.h
 * @brief Contains helper functions for the parser generator.
 * @author Vadim Melnikov
 * @version 1.0
 */
#pragma once

#include "Entities.h"

/**
 * @brief The epsilon (empty production) terminal.
 */
const Terminal EPSILON = Terminal{""};
/**
 * @brief The terminal that marks the end of the input.
 */
const Terminal T_EOF = Terminal{"$", "$"};

/**
 * @brief Helper function to check whether a token is a terminal or not.
 * @param token The token to check.
 * @return true if the token is a terminal, false otherwise.
 */
bool IsTerminal(const Token &token);
/**
 * @brief Helper function to check whether a token is a non-terminal or not.
 * @param token The token to check.
 * @return true if the token is a non-terminal, false otherwise.
 */
bool IsNonTerminal(const Token &token);

/**
 * @brief Returns a qualified name of the token.
 * @details Adds a prefix based on the nature of the token. Created to avoid
 * ambiguities for, say, a quote terminal and a regex terminal with same name.
 * @param token The token to get the qualified name for.
 * @return The qualified name of the token.
 */
std::string QualName(Token token);
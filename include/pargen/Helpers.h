#pragma once

#include "Entities.h"

const Terminal EPSILON = Terminal{""};
const Terminal T_EOF = Terminal{"$", "$"};

bool IsTerminal(const Token &token);
bool IsNonTerminal(const Token &token);

std::string QualName(Token token);
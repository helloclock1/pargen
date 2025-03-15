#pragma once

#include <cctype>
#include <cstring>
#include <iostream>
#include <istream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "Entities.h"

class GrammarParser {
public:
    GrammarParser(std::istream *in);

    void Parse();
    void Print();
    const Grammar &Get() const;

private:
    int GetChar();
    int GetChar(int expected);
    int Peek() const;
    bool PeekAt(int c) const;
    void SkipWS();

    void ParseLine();
    std::vector<Token> ParseRule();
    Token ParseToken();
    Terminal ParseQuoteTerminal();
    std::string ParseName();

    std::istream *in_;
    Grammar g_;
};
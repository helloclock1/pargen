#pragma once

#include <cctype>
#include <cstring>
#include <iostream>
#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "Entities.h"

class GrammarParserError : public std::exception {
public:
    explicit GrammarParserError(const std::string &msg, size_t line);

    const char *what() const noexcept override;

private:
    static std::string ConstructMessage(const std::string &msg, size_t line);
    std::string msg_;
};

class GrammarParser {
public:
    GrammarParser(std::unique_ptr<std::istream> in);

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
    void ParseIgnore();

    std::unique_ptr<std::istream> in_;
    size_t line_ = 0;
    Grammar g_;
};
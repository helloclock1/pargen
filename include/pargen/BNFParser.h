#pragma once

#include <cstring>
#include <iostream>
#include <istream>
#include <string>
#include <vector>

#include "Entities.h"

class GrammarParser {
public:
    GrammarParser(std::istream *in);

    void Parse();
    const Grammar &Get() const;

private:
    std::vector<std::string> Split(std::string s, const std::string &delim);
    std::string TrimWS(const std::string &s);

    bool IsTerminal(const std::string &s);
    void AssertIsTerminal(const std::string &s);
    bool IsNonTerminal(const std::string &s);
    void AssertIsNonTerminal(const std::string &s);

    void ParseLine(const std::string &s);

    std::istream *in_;
    Grammar g_;
};
#pragma once

#include <cstring>
#include <iostream>
#include <istream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Entities.h"

using GrammarGraph =
    std::unordered_map<NonTerminal, std::unordered_set<NonTerminal>>;

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

    void TopologicalSort();

    std::istream *in_;
    Grammar rules_;
};
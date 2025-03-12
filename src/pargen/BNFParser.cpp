#include "BNFParser.h"

GrammarParser::GrammarParser(std::istream *in) : in_(in) {
}

void GrammarParser::Parse() {
    std::string line;
    while (std::getline(*in_, line)) {
        ParseLine(line);
    }
}

const Grammar &GrammarParser::Get() const {
    return rules_;
}

std::vector<std::string> GrammarParser::Split(std::string s,
                                              const std::string &delim) {
    std::vector<std::string> result;
    std::string rule;
    size_t pos = s.find(delim);
    while (pos != std::string::npos) {
        result.push_back(TrimWS(s.substr(0, pos)));
        s.erase(0, pos + delim.length());
        pos = s.find(delim);
    }
    result.push_back(TrimWS(s));
    return result;
}

std::string GrammarParser::TrimWS(const std::string &s) {
    size_t first = s.find_first_not_of(' ');
    if (first == std::string::npos) {
        return "";
    }
    size_t last = std::min(s.size(), s.find_last_not_of(' '));
    return s.substr(first, last - first + 1);
}

bool GrammarParser::IsTerminal(const std::string &s) {
    if (s.front() != '<' && s.back() != '>') {
        return true;
    }
    return false;
}

void GrammarParser::AssertIsTerminal(const std::string &s) {
    if (s.front() == '<' || s.back() == '>') {
        throw std::runtime_error(
            "Expression `" + s +
            "` was expected to be a terminal but isn't one.");
    }
}

bool GrammarParser::IsNonTerminal(const std::string &s) {
    if (s.front() == '<' && s.back() == '>' &&
        s.find(' ') == std::string::npos) {
        return true;
    }
    return false;
}

void GrammarParser::AssertIsNonTerminal(const std::string &s) {
    if (!IsNonTerminal(s)) {
        throw std::runtime_error(
            "Expression `" + s +
            "` was expected to be a non-terminal but isn't one.");
    }
}

void GrammarParser::ParseLine(const std::string &s) {
    std::vector<std::string> sides = Split(s, "::=");
    if (sides.size() != 2) {
        throw std::runtime_error("Incorrect `::=` placement.");
    }
    std::string lhs = sides[0], rhs = sides[1];
    AssertIsNonTerminal(lhs);
    lhs = lhs.substr(1, lhs.length() - 2);
    auto production_list = Split(rhs, "|");
    for (const auto &raw_rule : production_list) {
        Production prod;
        std::vector<std::string> tokens = Split(raw_rule, " ");
        for (const auto &token : tokens) {
            if (IsTerminal(token)) {
                prod.push_back(Terminal{token});
            } else if (IsNonTerminal(token)) {
                prod.push_back(
                    NonTerminal{token.substr(1, token.length() - 2)});
            } else {
                throw std::runtime_error("Unrecognizable token `" + token +
                                         "`");
            }
        }
        rules_.push_back(Rule{NonTerminal{lhs}, prod});
    }
}

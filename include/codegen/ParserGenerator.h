#pragma once

#include "Automaton.h"
#include "Entities.h"

class ParserGeneratorError : public std::exception {
public:
    explicit ParserGeneratorError(const std::string &msg);
    const char *what() const noexcept override;

private:
    std::string msg_;
};

class ParserGenerator {
public:
    ParserGenerator(
        const std::string &folder, const Grammar &g, const ActionTable &at,
        const GotoTable &gt, const FollowSets &fs, bool add_json_generator,
        size_t json_indents
    );
    void Generate();

private:
    std::string folder_;
    const Grammar &g_;
    const ActionTable &at_;
    const GotoTable &gt_;
    const FollowSets &fs_;
    bool add_json_generator_;
    size_t json_indents_;
};
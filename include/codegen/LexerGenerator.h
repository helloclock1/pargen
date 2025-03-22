#pragma once

#include <string>

#include "Entities.h"

class LexerGeneratorError : public std::exception {
public:
    explicit LexerGeneratorError(const std::string &msg);
    const char *what() const noexcept override;

private:
    std::string msg_;
};

class LexerGenerator {
public:
    LexerGenerator(const std::string &folder, const Grammar &g);
    void Generate();

private:
    std::string folder_;
    const Grammar &g_;
};
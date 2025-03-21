#include <cstdlib>
#include <cstring>

#include "Automaton.h"

class CodeGeneratorError : public std::exception {
public:
    explicit CodeGeneratorError(const std::string &msg);

    const char *what() const noexcept override;

private:
    std::string msg_;
};

class CodeGenerator {
public:
    CodeGenerator(
        const std::string &folder, ActionTable &at, GotoTable &gt,
        FollowSets &fs, const Grammar &g, bool add_json_generator,
        size_t json_indents
    );
    void Generate();

private:
    void GenerateLexer();
    void GenerateParser();

    std::string folder_;

    Grammar g_;
    ActionTable &at_;
    GotoTable &gt_;
    FollowSets &fs_;

    bool add_json_generator_;
    size_t json_indents_;
};
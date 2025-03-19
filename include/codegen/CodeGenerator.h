#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "Automaton.h"

class CodeGenerator {
public:
    CodeGenerator(
        const std::string &folder, ActionTable &at, GotoTable &gt,
        const Grammar &g
    );
    void Generate();

private:
    void GenerateLexer();
    void GenerateParser();

    std::string folder_;

    Grammar g_;
    ActionTable &at_;
    GotoTable &gt_;
};
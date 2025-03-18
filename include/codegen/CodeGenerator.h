#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

#include "Automaton.h"

class CodeGenerator {
public:
    CodeGenerator(const std::string &folder, ActionTable &at, GotoTable &gt,
                  const Grammar &g);
    void Generate();

private:
    void GenerateStructure();
    void GenerateTypes();
    void GenerateLexer();
    void GenerateParser();
    void GenerateMain();
    void GenerateCMakeLists();

    std::string folder_;

    Grammar g_;
    ActionTable &at_;
    GotoTable &gt_;
};
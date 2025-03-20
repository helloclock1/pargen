#include <cstring>
#include <fstream>
#include <string>

#include "Automaton.h"
#include "BNFParser.h"
#include "CodeGenerator.h"
#include "Entities.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: ./gen rules.bnf\n";
        return 0;
    }
    std::string filename = argv[1];
    std::ifstream in(filename);
    GrammarParser gp(&in);
    try {
        gp.Parse();
    } catch (const GrammarParserError &e) {
        std::cerr << "GrammarParserError" << e.what() << std::endl;
        return 1;
    }
    Grammar g = gp.Get();
    ParserGenerator a(g);
    ActionTable at = a.GetActionTable();
    GotoTable gt = a.GetGotoTable();
    CodeGenerator codegen("parser", at, gt, g);
    codegen.Generate();
    return 0;
}

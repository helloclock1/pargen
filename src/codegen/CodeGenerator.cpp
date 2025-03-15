#include "CodeGenerator.h"

#include "Automaton.h"
#include "Entities.h"

CodeGenerator::CodeGenerator(const std::string &folder, ActionTable &at,
                             GotoTable &gt, const Grammar &g)
    : folder_(folder), at_(at), gt_(gt), g_(g) {
}

void CodeGenerator::Generate() {
    GenerateStructure();
    GenerateTypes();
    GenerateLexer();
    GenerateParser();
    GenerateMain();
    GenerateCMakeLists();
}

void CodeGenerator::GenerateStructure() {
    std::filesystem::create_directories(folder_ + "/src");
    std::filesystem::create_directories(folder_ + "/include");
    std::filesystem::create_directories(folder_ + "/build");
    std::filesystem::create_directories(folder_ + "/apps");
    std::filesystem::create_directories(folder_ + "/tmp");
}

void CodeGenerator::GenerateTypes() {
    // TODO(helloclock): there are definitely redundant functions generated here
    std::ofstream out(folder_ + "/include/Types.h");
    out << "#pragma once\n";
    out << "\n";
    out << "#include <optional>\n";
    out << "#include <set>\n";
    out << "#include <string>\n";
    out << "#include <unordered_map>\n";
    out << "#include <variant>\n";
    out << "#include <vector>\n";
    out << "\n";
    out << "struct Terminal {\n";
    out << "    std::string name;\n";
    out << "    std::string repr = \"\";\n";
    out << "    bool operator==(const Terminal &other) const;\n";
    out << "    bool operator!=(const Terminal &other) const;\n";
    out << "};\n";
    out << "\n";
    out << "struct NonTerminal {\n";
    out << "    std::string name;\n";
    out << "    bool operator==(const NonTerminal &other) const;\n";
    out << "    bool operator!=(const NonTerminal &other) const;\n";
    out << "};\n";
    out << "\n";
    out << "using Token = std::variant<Terminal, NonTerminal>;\n";
    out << "\n";
    out << "bool IsTerminal(const Token &token);\n";
    out << "bool IsNonTerminal(const Token &token);\n";
    out << "\n";
    out << "namespace std {\n";
    out << "template <>\n";
    out << "struct hash<Terminal> {\n";
    out << "    size_t operator()(const Terminal &t) const {\n";
    out << "        return hash<string>()(\"t\" + t.name);\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "template <>\n";
    out << "struct hash<NonTerminal> {\n";
    out << "    size_t operator()(const NonTerminal &nt) const {\n";
    out << "        return hash<string>()(\"nt\" + nt.name);\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "template <>\n";
    out << "struct hash<Token> {\n";
    out << "    size_t operator()(const Token &t) const {\n";
    out << "        if (IsTerminal(t)) {\n";
    out << "            return hash<Terminal>()(std::get<Terminal>(t));\n";
    out << "        } else {\n";
    out << "            return "
           "hash<NonTerminal>()(std::get<NonTerminal>(t));\n";
    out << "        }\n";
    out << "    }\n";
    out << "};\n";
    out << "};  // namespace std\n";
    out << "\n";
    out << "bool operator<(const Token &a, const Token &b);\n";
    out << "\n";
    out << "using Production = std::vector<Token>;\n";
    out << "\n";
    out << "struct Rule {\n";
    out << "    NonTerminal lhs;\n";
    out << "    Production prod;\n";
    out << "};\n";
    out << "\n";
    out << "using Grammar = std::vector<Rule>;\n";
    out << "\n";
    out << "struct Item {\n";
    out << "    Item(size_t rule_number, size_t dot_pos);\n";
    out << "    size_t rule_number;\n";
    out << "    size_t dot_pos;\n";
    out << "\n";
    out << "    friend bool operator<(const Item &a, const Item &b);\n";
    out << "    bool DotAtEnd() const;\n";
    out << "    std::optional<Token> NextToken() const;\n";
    out << "    Rule GetRule() const;\n";
    out << "};\n";
    out << "\n";
    out << "using State = std::set<Item>;\n";
    out << "\n";
    out << "enum class ActionType {\n";
    out << "    SHIFT,\n";
    out << "    REDUCE,\n";
    out << "    ACCEPT,\n";
    out << "    ERROR\n";
    out << "};\n";
    out << "\n";
    out << "struct Action {\n";
    out << "    ActionType type = ActionType::ERROR;\n";
    out << "    size_t value = 0;\n";
    out << "};\n";
    out << "\n";
    out << "using ActionTable = std::vector<std::unordered_map<std::string, "
           "Action>>;\n";
    out << "using GotoTable = std::unordered_map<size_t, "
           "std::unordered_map<NonTerminal, size_t>>;\n";
    out.close();

    out = std::ofstream(folder_ + "/src/Types.cpp");
    out << "#include \"Types.h\"\n";
    out << "\n";
    out << "bool Terminal::operator==(const Terminal &other) const {\n";
    out << "    return name == other.name;\n";
    out << "}\n";
    out << "\n";
    out << "bool Terminal::operator!=(const Terminal &other) const {\n";
    out << "    return name != other.name;\n";
    out << "}\n";
    out << "\n";
    out << "bool NonTerminal::operator==(const NonTerminal &other) const {\n";
    out << "    return name == other.name;\n";
    out << "}\n";
    out << "\n";
    out << "bool NonTerminal::operator!=(const NonTerminal &other) const {\n";
    out << "    return name != other.name;\n";
    out << "}\n";
    out.close();
}

void CodeGenerator::GenerateLexer() {
    std::ofstream out(folder_ + "/tmp/Lexer.l");
    out << "%option noyywrap\n";
    out << "%{\n";
    out << "#include <cstdio>\n";
    out << "#include <cstdlib>\n";
    out << "#include <iostream>\n";
    out << "#include <vector>\n";
    out << "\n";
    out << "#include \"Types.h\"\n";
    out << "\n";
    out << "std::vector<Terminal> tokens;\n";
    out << "\n";
    out << "extern FILE *yyin;\n";
    out << "extern \"C\" int yylex();\n";
    out << "\n";
    out << "std::vector<Terminal> Lex(const char *filename) {\n";
    out << "    FILE *file = fopen(filename, \"r\");\n";
    out << "    if (!file) {\n";
    out << "        std::cerr << \"Error: cannot open file `\" << filename << "
           "\"`\" << std::endl;\n";
    out << "        exit(1);\n";
    out << "    }\n";
    out << "    yyin = file;\n";
    out << "    yylex();\n";
    out << "    return tokens;\n";
    out << "}\n";
    out << "%}\n";
    out << "\n";
    out << "%%\n";
    out << "\n";
    for (const Token &token : g_.tokens_) {
        if (IsTerminal(token)) {
            Terminal t = std::get<Terminal>(token);
            if (t.name_.empty()) {
                continue;
            }
            if (t.repr_.empty()) {
                out << "\"";
                for (const char &c : t.name_) {
                    // TODO(helloclock): check for existence of other escape
                    // chars
                    if (c == '"' || c == '\\') {
                        out << "\\";
                    }
                    out << c;
                }
                out << "\"" << "\t"
                    << "{ tokens.push_back(Terminal{yytext}); }\n";
            } else {
                out << t.repr_ << "\t" << "{ tokens.push_back(Terminal{\""
                    << t.name_ << "\", yytext}); }\n";
            }
        }
    }

    out << "\n";
    out << "%%\n";
    out.close();

    std::string build_lexer_command = "flex --outfile=" + folder_ +
                                      "/src/Lexer.cpp " + folder_ +
                                      "/tmp/Lexer.l";
    system(build_lexer_command.c_str());
    std::filesystem::remove_all(folder_ + "/tmp");
}

void CodeGenerator::GenerateParser() {
    std::ofstream out(folder_ + "/include/Parser.h");
    out << "#pragma once\n";
    out << "\n";
    out << "#include <ranges>\n";
    out << "#include <stack>\n";
    out << "#include <vector>\n";
    out << "\n";
    out << "#include \"Types.h\"\n";
    out << "\n";
    out << "std::string QualName(Token token);\n";
    out << "\n";
    out << "class ParserTables {\n";
    out << "public:\n";
    out << "    static const ActionTable &GetActionTable() {\n";
    out << "        static const ActionTable table = {\n";
    for (size_t i = 0; i < at_.size(); ++i) {
        out << "            {\n";
        size_t j = 0;
        for (const auto &[terminal, action] : at_[i]) {
            out << "                ";
            out << "{\"" << terminal << "\", Action{ActionType::";
            switch (action.type_) {
                case ActionType::ACCEPT:
                    out << "ACCEPT";
                    break;
                case ActionType::ERROR:
                    out << "ERROR";
                    break;
                case ActionType::REDUCE:
                    out << "REDUCE";
                    break;
                case ActionType::SHIFT:
                    out << "SHIFT";
                    break;
            }
            out << ", " << action.value_ << "}}";
            if (j != at_[i].size() - 1) {
                out << ",";
            }
            ++j;
            out << "\n";
        }
        out << "            }";
        if (i != at_.size() - 1) {
            out << ",";
        }
        out << "\n";
    }
    out << "        };\n";
    out << "\n";
    out << "        return table;\n";
    out << "    }\n";
    out << "\n";
    out << "    static const GotoTable &GetGotoTable() {\n";
    out << "        static const GotoTable table = {\n";
    for (const auto &[state, table] : gt_) {
        size_t nonzero_count = 0;
        for (const auto &[terminal, goto_value] : table) {
            if (goto_value != 0) {
                ++nonzero_count;
            }
        }
        if (nonzero_count == 0) {
            continue;
        }
        out << "            ";
        out << "{\n";
        out << "                ";
        out << state << ", {\n";
        size_t j = 0;
        for (const auto &[terminal, goto_value] : table) {
            if (goto_value == 0) {
                continue;
            }
            out << "                    ";
            out << "{NonTerminal{\"" << terminal.name_ << "\"}, " << goto_value
                << "}";
            if (j != table.size() - 1) {
                out << ",";
            }
            ++j;
            out << "\n";
        }
        out << "                }\n";
        out << "            },\n";
    }
    out << "        };\n";
    out << "\n";
    out << "        return table;\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "class Parser {\n";
    out << "public:\n";
    out << "    Parser();\n";
    out << "    void Parse(const std::vector<Terminal> &stream);\n";
    out << "\n";
    out << "private:\n";
    out << "    void Clear();\n";
    out << "\n";
    out << "    Grammar g_ = {\n";
    for (const Rule &rule : g_.rules_) {
        out << "        {\n";
        out << "            NonTerminal{\"" << rule.lhs.name_ << "\"},\n";
        out << "            {\n";
        for (const Token &token : rule.prod) {
            out << "                ";
            if (IsTerminal(token)) {
                Terminal t = std::get<Terminal>(token);
                out << "Terminal{\"" << t.name_ << "\"";
                if (!t.repr_.empty()) {
                    out << ", \"" << t.repr_ << "\"";
                }
                out << "},\n";
            } else {
                out << "NonTerminal{\"" << std::get<NonTerminal>(token).name_
                    << "\"},\n";
            }
        }
        out << "            },\n";
        out << "        },\n";
    }
    out << "    };\n";
    out << "\n";
    out << "    std::stack<Terminal> seq_;\n";
    out << "    std::stack<size_t> state_stack_;\n";
    out << "    std::stack<Token> symbol_stack_;\n";
    out << "\n";
    out << "    ActionTable action_ = ParserTables::GetActionTable();\n";
    out << "    GotoTable goto_ = ParserTables::GetGotoTable();\n";
    out << "};\n";
    out.close();
    out = std::ofstream(folder_ + "/src/Parser.cpp");
    out << "#include \"Parser.h\"\n";
    out << "#include \"Types.h\"";
    out << "\n";
    out << "#include <iostream>\n";
    out << "#include <stdexcept>\n";
    out << "\n";
    out << "std::string QualName(Token token) {\n";
    out << "    if (std::holds_alternative<Terminal>(token)) {\n";
    out << "        Terminal t = std::get<Terminal>(token);\n";
    out << "        if (t.repr.empty()) {\n";
    out << "            return \"T_\" + t.name;\n";
    out << "        } else {\n";
    out << "            return \"R_\" + t.name;\n";
    out << "        }\n";
    out << "    } else {\n";
    out << "        return \"NT_\" + std::get<NonTerminal>(token).name;\n";
    out << "    }\n";
    out << "}\n";
    out << "\n";
    out << "Parser::Parser() {\n";
    out << "}\n";
    out << "\n";
    // TODO(helloclock): add epsilon-string support
    out << "void Parser::Parse(const std::vector<Terminal> &stream) {\n";
    out << "    Clear();\n";
    out << "    for (const Terminal &token : stream | std::views::reverse) {\n";
    out << "        seq_.push(token);\n";
    out << "    }\n";
    out << "\n";
    out << "    Terminal a = seq_.top();\n";
    out << "    bool done = false;\n";
    out << "    while (!done) {\n";
    out << "        size_t s = state_stack_.top();\n";
    out << "        Action action = action_[s][QualName(a)];\n";
    out << "        switch (action.type) {\n";
    out << "            case ActionType::SHIFT:\n";
    out << "                symbol_stack_.push(a);\n";
    out << "                state_stack_.push(action.value);\n";
    out << "                seq_.pop();\n";
    out << "                a = seq_.top();\n";
    out << "                break;\n";
    out << "            case ActionType::REDUCE: {\n";
    out << "                Rule rule = g_[action.value];\n";
    out << "                if (QualName(rule.prod[0]) != \"T_\") {\n";
    out << "                    for (size_t i = 0; i < rule.prod.size(); ++i) "
           "{\n";
    out << "                        symbol_stack_.pop();\n";
    out << "                        state_stack_.pop();\n";
    out << "                    }\n";
    out << "                }\n";
    out << "                size_t t = state_stack_.top();\n";
    out << "                symbol_stack_.push(rule.lhs);\n";
    out << "                state_stack_.push(goto_[t][rule.lhs]);\n";
    out << "                break;\n";
    out << "            }\n";
    out << "            case ActionType::ACCEPT:\n";
    out << "                done = true;\n";
    out << "                break;\n";
    out << "            case ActionType::ERROR:\n";
    out << "                throw std::runtime_error(\"PANIC PANIC PANIC\");\n";
    out << "        }\n";
    out << "    }\n";
    out << "}\n";
    out << "\n";
    out << "void Parser::Clear() {\n";
    out << "    while (!seq_.empty()) {\n";
    out << "        seq_.pop();\n";
    out << "    }\n";
    out << "    seq_.push(Terminal{\"$\"});\n";
    out << "    while (!state_stack_.empty()) {\n";
    out << "        state_stack_.pop();\n";
    out << "    }\n";
    out << "    state_stack_.push(0);\n";
    out << "    while (!symbol_stack_.empty()) {\n";
    out << "        symbol_stack_.pop();\n";
    out << "    }\n";
    out << "}\n";
    out.close();
}

void CodeGenerator::GenerateMain() {
    std::ofstream out(folder_ + "/apps/main.cpp");
    out << "#include \"Parser.h\"\n";
    out << "#include \"Types.h\"\n";
    out << "\n";
    out << "// This is an example lexer\n";
    out << "// Feel free to use any other lexer that would lex a stream of "
           "tokens\n";
    out << "// to a sequence of `Terminal`.\n";
    out << "extern std::vector<Terminal> Lex(const char *filename);\n";
    out << "\n";
    out << "int main () {\n";
    out << "    const char *filename = \"file\";\n";
    out << "    std::vector<Terminal> stream = Lex(filename);\n";
    out << "    Parser parser;\n";
    out << "    parser.Parse(stream);\n";
    out << "    return 0;\n";
    out << "}\n";
    out.close();
}

void CodeGenerator::GenerateCMakeLists() {
    std::ofstream out(folder_ + "/CMakeLists.txt");
    out << "cmake_minimum_required(VERSION 3.31)\n";
    out << "project(Parser LANGUAGES CXX)\n";
    out << "\n";
    out << "set(CMAKE_CXX_STANDARD 20)\n";
    out << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    out << "\n";
    out << "add_library(parser_lib src/Types.cpp src/Parser.cpp "
           "src/Lexer.cpp)\n";
    out << "\n";
    out << "target_include_directories(parser_lib PUBLIC "
           "${CMAKE_CURRENT_SOURCE_DIR}/include)\n";
    out << "\n";
    out << "add_executable(parser apps/main.cpp)\n";
    out << "\n";
    out << "target_link_libraries(parser PRIVATE parser_lib)\n";
    out << "target_compile_options(parser PRIVATE -Werror -Wall -Wextra "
           "-Wpedantic)\n";
    out.close();
}
#include "LexerGenerator.h"

#include <filesystem>
#include <fstream>

#include "Helpers.h"

LexerGeneratorError::LexerGeneratorError(const std::string &msg) : msg_(msg) {
}

const char *LexerGeneratorError::what() const noexcept {
    return msg_.c_str();
}

LexerGenerator::LexerGenerator(const std::string &folder, const Grammar &g)
    : folder_(folder), g_(g) {
}

void LexerGenerator::Generate() {
    std::ofstream out(folder_ + "/Lexer.l");
    out << "%option noyywrap\n";
    out << "%{\n";
    out << "#include <cstdio>\n";
    out << "#include <cstdlib>\n";
    out << "#include <iostream>\n";
    out << "#include <vector>\n";
    out << "\n";
    out << "#include \"LexerFwd.hpp\"\n";
    out << "\n";
    out << "std::vector<p::Terminal> tokens;\n";
    out << "\n";
    out << "extern FILE *yyin;\n";
    out << "extern \"C\" int yylex();\n";
    out << "\n";
    out << "std::vector<p::Terminal> Lex(const char *filename) {\n";
    out << "    tokens.clear();\n";
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
            if (t == T_EOF || t.name_.empty()) {
                continue;
            }
            if (t.IsQuote()) {
                out << "\"";
                for (const char &c : t.name_) {
                    // user may pass something like '\n' as a token. this should
                    // be taken exactly as if it was embraced in double quotes
                    // without modifying (so '\n' is considered a newline, not a
                    // sequence of backslash and `n`)
                    if (c == '"') {
                        out << "\\";
                    }
                    out << c;
                }
                out << "\"" << "\t"
                    << "{ tokens.push_back(p::Terminal{yytext}); }\n";
            } else if (t.repr_ != " ") {
                out << t.repr_ << "\t" << "{ tokens.push_back(p::Terminal{\""
                    << t.name_ << "\", yytext}); }\n";
            }
        }
    }
    for (const std::string &regex : g_.ignored_) {
        out << regex << "\t;\n";
    }

    out << "\n";
    out << "%%\n";
    out.close();

    std::string build_lexer_command = "flex --outfile=" + folder_ +
                                      "/Lexer.cpp " + folder_ +
                                      "/Lexer.l > /dev/null 2>&1";
    int result = std::system(build_lexer_command.c_str());
    if (result != 0) {
        throw LexerGeneratorError(
            "Failed to build lexer. Check whether `flex` is installed and "
            "available in PATH."
        );
    }
    std::filesystem::remove(folder_ + "/Lexer.l");
    out = std::ofstream(folder_ + "/LexerFwd.hpp");
    out << "#pragma once\n";
    out << "\n";
    out << "#include \"variant\"\n";
    out << "\n";
    out << "namespace p {\n";
    out << "struct Terminal {\n";
    out << "    std::string name;\n";
    out << "    std::string repr = \"\";\n";
    out << "    bool operator==(const Terminal &other) const {\n";
    out << "        return name == other.name;\n";
    out << "    }\n";
    out << "    bool operator!=(const Terminal &other) const {\n";
    out << "        return name != other.name;\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "struct NonTerminal {\n";
    out << "    std::string name;\n";
    out << "    bool operator==(const NonTerminal &other) const {\n";
    out << "        return name == other.name;\n";
    out << "    }\n";
    out << "    bool operator!=(const NonTerminal &other) const {\n";
    out << "        return name != other.name;\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "using Token = std::variant<Terminal, NonTerminal>;\n";
    out << "};  // namespace p\n";
}
#include "ParserGenerator.h"

#include <fstream>
#include <iostream>

#include "Helpers.h"

ParserGeneratorError::ParserGeneratorError(const std::string &msg) : msg_(msg) {
}

const char *ParserGeneratorError::what() const noexcept {
    return msg_.c_str();
}

ParserGenerator::ParserGenerator(
    const std::string &folder, const Grammar &g, const ActionTable &at,
    const GotoTable &gt, const FollowSets &fs, bool add_json_generator,
    size_t json_indents
)
    : folder_(folder),
      g_(g),
      at_(at),
      gt_(gt),
      fs_(fs),
      add_json_generator_(add_json_generator),
      json_indents_(json_indents) {
}

void ParserGenerator::Generate() {
    std::ofstream out(folder_ + "/Parser.hpp");
    out << "#pragma once\n";
    out << "\n";
    out << "#include <algorithm>\n";
    out << "#include <iostream>\n";
    out << "#include <fstream>\n";
    out << "#include <memory>\n";
    if (add_json_generator_) {
        out << "#include <nlohmann/json.hpp>\n";
    }
    out << "#include <ranges>\n";
    out << "#include <set>\n";
    out << "#include <stack>\n";
    out << "#include <stdexcept>\n";
    out << "#include <string>\n";
    out << "#include <unordered_map>\n";
    out << "#include <variant>\n";
    out << "#include <vector>\n";
    out << "\n";
    out << "#include \"LexerFwd.hpp\"\n";
    out << "\n";
    out << "namespace std {\n";
    out << "template <>\n";
    out << "struct hash<p::Terminal> {\n";
    out << "    size_t operator()(const p::Terminal &t) const {\n";
    out << "        return hash<std::string>()(\"t\" + t.name);\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "template <>\n";
    out << "struct hash<p::NonTerminal> {\n";
    out << "    size_t operator()(const p::NonTerminal &nt) const {\n";
    out << "        return hash<std::string>()(\"nt\" + nt.name);\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "template <>\n";
    out << "struct hash<p::Token> {\n";
    out << "    size_t operator()(const p::Token &t) const {\n";
    out << "        if (std::holds_alternative<p::Terminal>(t)) {\n";
    out << "            return "
           "hash<p::Terminal>()(std::get<p::Terminal>(t));\n";
    out << "        } else {\n";
    out << "            return "
           "hash<p::NonTerminal>()(std::get<p::NonTerminal>(t));\n";
    out << "        }\n";
    out << "    }\n";
    out << "};\n";
    out << "};  // namespace std\n";
    out << "\n";
    out << "namespace p {\n";
    out << "bool operator<(const Terminal &lhs, const Terminal &rhs) {\n";
    out << "    return lhs.name < rhs.name;\n";
    out << "}\n";
    out << "using Production = std::vector<Token>;\n";
    out << "\n";
    out << "struct Rule {\n";
    out << "    NonTerminal lhs;\n";
    out << "    Production prod;\n";
    out << "};\n";
    out << "\n";
    out << "using Grammar = std::vector<Rule>;\n";
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
    out << "using FollowSet = std::set<Terminal>;\n";
    out << "using FollowSets = std::unordered_map<NonTerminal, FollowSet>;\n";
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
    out << "\n";
    out << "    static const FollowSet GetFollowSetFor(const NonTerminal &nt) "
           "{\n";
    out << "        return GetFollowSets().at(nt);\n";
    out << "    }\n";
    out << "\n";
    out << "private:\n";
    out << "    static const FollowSets GetFollowSets() {\n";
    out << "        static const FollowSets table = {\n";
    for (const auto &[nt, follow_set] : fs_) {
        out << "            ";
        out << "{NonTerminal{\"" << nt.name_ << "\"}, {\n";
        size_t j = 0;
        for (const auto &follow_t : follow_set) {
            out << "                ";
            out << "Terminal{\"" << follow_t.name_ << "\"";
            if (!follow_t.repr_.empty()) {
                out << ", \"" << follow_t.repr_ << "\"";
            }
            out << "}";
            if (j != follow_set.size() - 1) {
                out << ",";
            }
            ++j;
            out << "\n";
        }
        out << "            }},\n";
    }
    out << "        };\n";
    out << "        return table;\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "class ParseTreePreorderVisitor {\n";
    out << "public:\n";
    out << "    virtual void VisitTerminal(const Terminal &t) = 0;\n";
    out << "    virtual void VisitNonTerminal(const NonTerminal &nt) = 0;\n";
    out << "    virtual ~ParseTreePreorderVisitor() = default;\n";
    out << "};\n";
    out << "\n";
    out << "class ParseTreePostorderVisitor {\n";
    out << "public:\n";
    out << "    virtual void VisitTerminal(const Terminal &t) = 0;\n";
    out << "    virtual void VisitNonTerminal(const NonTerminal &nt) = 0;\n";
    out << "    virtual ~ParseTreePostorderVisitor() = default;\n";
    out << "};\n";
    out << "\n";
    out << "struct ParseTreeNode {\n";
    out << "    Token value;\n";
    out << "    std::vector<std::shared_ptr<ParseTreeNode>> children;\n";
    out << "\n";
    out << "    void Accept(ParseTreePreorderVisitor &visitor) const "
           "{\n";
    out << "        if (std::holds_alternative<p::Terminal>(value)) {\n";
    out << "            visitor.VisitTerminal(std::get<Terminal>(value));\n";
    out << "        } else {\n";
    out << "            "
           "visitor.VisitNonTerminal(std::get<NonTerminal>(value));\n";
    out << "        }\n";
    out << "        for (const auto &child : children) {\n";
    out << "            child->Accept(visitor);\n";
    out << "        }\n";
    out << "    }\n";
    out << "\n";
    out << "    void Accept(ParseTreePostorderVisitor &visitor) const {\n";
    out << "        for (const auto &child : children) {\n";
    out << "            child->Accept(visitor);\n";
    out << "        }\n";
    out << "        if (std::holds_alternative<p::Terminal>(value)) {\n";
    out << "            visitor.VisitTerminal(std::get<Terminal>(value));\n";
    out << "        } else {\n";
    out << "            "
           "visitor.VisitNonTerminal(std::get<NonTerminal>(value));\n";
    out << "        }\n";
    out << "    }\n";
    out << "};\n";
    out << "\n";
    out << "class ParseTree {\n";
    out << "public:\n";
    out << "    explicit ParseTree(std::shared_ptr<ParseTreeNode> root) : "
           "root_(root) {}\n";
    out << "\n";
    out << "    std::shared_ptr<ParseTreeNode> GetRoot() const {\n";
    out << "        return root_;\n";
    out << "    }\n";
    out << "\n";
    out << "    void Accept(ParseTreePreorderVisitor &visitor) const {\n";
    out << "        root_->Accept(visitor);\n";
    out << "    }\n";
    out << "\n";
    out << "    void Accept(ParseTreePostorderVisitor &visitor) const {\n";
    out << "        root_->Accept(visitor);\n";
    out << "    }\n";
    out << "\n";
    out << "private:\n";
    out << "    std::shared_ptr<ParseTreeNode> root_;\n";
    out << "};\n";
    out << "\n";
    if (add_json_generator_) {
        out << "using json = nlohmann::ordered_json;\n";
        out << "\n";
        out << "class JsonTreeGenerator {\n";
        out << "public:\n";
        out << "    JsonTreeGenerator(const std::string &filename) : "
               "filename_(filename) {}\n";
        out << "\n";
        out << "    void Generate(ParseTree tree) {\n";
        out << "        json j = GenerateForNode(tree.GetRoot());\n";
        out << "        std::ofstream out(filename_);\n";
        out << "        out << j.dump(" << std::to_string(json_indents_)
            << ");\n";
        out << "    }\n";
        out << "\n";
        out << "private:\n";
        out << "    json GenerateForNode(std::shared_ptr<ParseTreeNode> "
               "node){\n";
        out << "        json tree;\n";
        out << "        if (std::holds_alternative<Terminal>(node->value)) {\n";
        out << "            Terminal t = std::get<Terminal>(node->value);\n";
        out << "            tree[\"value\"] = t.name;\n";
        out << "            if (!t.repr.empty()) {\n";
        out << "                tree[\"lexeme\"] = t.repr;\n";
        out << "            }\n";
        out << "        } else {\n";
        out << "            tree[\"type\"] = "
               "std::get<NonTerminal>(node->value).name;\n";
        out << "        }\n";
        out << "        if (node->children.empty()) {\n";
        out << "            return tree;\n";
        out << "        }\n";
        out << "        for (const auto& child : node->children) {\n";
        out << "            "
               "tree[\"children\"].push_back(GenerateForNode(child));\n";
        out << "        }\n";
        out << "        return tree;\n";
        out << "    }\n";
        out << "\n";
        out << "    std::string filename_;\n";
        out << "};\n";
        out << "\n";
    }
    out << "class Parser {\n";
    out << "public:\n";
    out << "    int Parse(const std::vector<Terminal> &stream) {\n";
    out << "        Clear();\n";
    out << "        for (const Terminal &token : stream | std::views::reverse) "
           "{\n";
    out << "            seq_.push(token);\n";
    out << "        }\n";
    out << "    \n";
    out << "        Terminal a = seq_.top();\n";
    out << "        bool done = false;\n";
    out << "        int return_state = 0;\n";
    out << "        while (!done) {\n";
    out << "            size_t s = state_stack_.top();\n";
    out << "            auto entry = action_.at(s);\n";
    out << "            Action action;\n";
    out << "            if (entry.find(QualName(a)) != entry.end()) {\n";
    out << "                action = entry.at(QualName(a));\n";
    out << "            } else {\n";
    out << "                action = Action{ActionType::ERROR};\n";
    out << "            }\n";
    out << "            switch (action.type) {\n";
    out << "                case ActionType::SHIFT: {\n";
    out << "                    auto new_node = "
           "std::make_shared<ParseTreeNode>(ParseTreeNode{a, {}});\n";
    out << "                    node_stack_.push(new_node);\n";
    out << "                    state_stack_.push(action.value);\n";
    out << "                    seq_.pop();\n";
    out << "                    a = seq_.top();\n";
    out << "                    break;\n";
    out << "                }\n";
    out << "                case ActionType::REDUCE: {\n";
    out << "                    Rule rule = g_[action.value];\n";
    out << "                    std::vector<std::shared_ptr<ParseTreeNode>> "
           "new_children;\n";
    out << "                    if (QualName(rule.prod[0]) != \"T_\") {\n";
    out << "                        for (size_t i = 0; i < rule.prod.size(); "
           "++i) "
           "{\n";
    out << "                            "
           "new_children.push_back(node_stack_.top());\n";
    out << "                            node_stack_.pop();\n";
    out << "                            state_stack_.pop();\n";
    out << "                        }\n";
    out << "                    }\n";
    out << "                    std::reverse(new_children.begin(), "
           "new_children.end());\n";
    out << "                    auto new_node = "
           "std::make_shared<ParseTreeNode>(ParseTreeNode{rule.lhs, "
           "new_children});\n";
    out << "                    node_stack_.push(new_node);\n";
    out << "                    size_t t = state_stack_.top();\n";
    out << "                    state_stack_.push(goto_.at(t).at(rule.lhs));\n";
    out << "                    current_nt_ = rule.lhs;\n";
    out << "                    break;\n";
    out << "                }\n";
    out << "                case ActionType::ACCEPT:\n";
    out << "                    done = true;\n";
    out << "                    break;\n";
    out << "                case ActionType::ERROR: {\n";
    out << "                    std::cerr << \"Error on token \" << "
           "(a.repr.empty() ? a.name : a.repr) << "
           "\", trying to recover\" << "
           "std::endl;\n";
    out << "                    ++return_state;\n";
    out << "                    FollowSet follow;\n";
    out << "                    try {\n";
    out << "                        follow = "
           "ParserTables::GetFollowSetFor(current_nt_);\n";
    out << "                    } catch (const std::out_of_range &e) {\n";
    out << "                        std::cerr << \"Error, cannot recover\" << "
           "std::endl;\n";
    out << "                        return -return_state;\n";
    out << "                    }\n";
    out << "                    bool recovered = false;\n";
    out << "                    while (!seq_.empty() && !recovered) {\n";
    out << "                        if (follow.find(seq_.top()) != "
           "follow.end()) {\n";
    out << "                            recovered = true;\n";
    out << "                        }\n";
    out << "                        seq_.pop();\n";
    out << "                    }\n";
    out << "                    if (!recovered) {\n";
    out << "                        std::cerr << \"Error, cannot recover\" << "
           "std::endl;\n";
    out << "                        return -return_state;\n";
    out << "                    }\n";
    out << "                    if (!seq_.empty()) {\n";
    out << "                        a = seq_.top();\n";
    out << "                    }\n";
    out << "                }\n";
    out << "            }\n";
    out << "        }\n";
    out << "        return return_state;\n";
    out << "    }\n";
    out << "\n";
    out << "    ParseTree GetParseTree() const {\n";
    out << "        return ParseTree(node_stack_.top());\n";
    out << "    }\n";
    out << "\n";
    out << "private:\n";
    out << "    void Clear() {\n";
    out << "        while (!seq_.empty()) {\n";
    out << "            seq_.pop();\n";
    out << "        }\n";
    out << "        seq_.push(Terminal{\"$\", \"$\"});\n";
    out << "        while (!state_stack_.empty()) {\n";
    out << "            state_stack_.pop();\n";
    out << "        }\n";
    out << "        state_stack_.push(0);\n";
    out << "        while (!node_stack_.empty()) {\n";
    out << "            node_stack_.pop();\n";
    out << "        }\n";
    out << "    }\n";
    out << "\n";
    out << "    inline static const Grammar g_ = {\n";
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
    out << "    std::string QualName(const Token& token) {\n";
    out << "        if (std::holds_alternative<Terminal>(token)) {\n";
    out << "            Terminal t = std::get<Terminal>(token);\n";
    out << "            if (t.repr.empty()) {\n";
    out << "                return \"T_\" + t.name;\n";
    out << "            } else {\n";
    out << "                return \"R_\" + t.name;\n";
    out << "            }\n";
    out << "        } else {\n";
    out << "            return \"NT_\" + std::get<NonTerminal>(token).name;\n";
    out << "        }\n";
    out << "    }\n";
    out << "\n";
    out << "    std::stack<Terminal> seq_;\n";
    out << "    std::stack<size_t> state_stack_;\n";
    out << "    std::stack<std::shared_ptr<ParseTreeNode>> node_stack_;\n";
    out << "\n";
    out << "    NonTerminal current_nt_;\n";
    out << "\n";
    out << "    inline static const ActionTable action_ = "
           "ParserTables::GetActionTable();\n";
    out << "    inline static const GotoTable goto_ = "
           "ParserTables::GetGotoTable();\n";
    out << "};\n";
    out << "};  // namespace p\n";
    out.close();
}
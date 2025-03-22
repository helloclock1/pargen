#include "CodeGenerator.h"

#include <exception>
#include <filesystem>

#include "Entities.h"
#include "LexerGenerator.h"
#include "ParserGenerator.h"

CodeGeneratorError::CodeGeneratorError(const std::string &msg) : msg_(msg) {
}

const char *CodeGeneratorError::what() const noexcept {
    return msg_.c_str();
}

CodeGenerator::CodeGenerator(
    const std::string &folder, ActionTable &at, GotoTable &gt, FollowSets &fs,
    const Grammar &g, bool add_json_generator, size_t json_indents
)
    : folder_(
          folder.starts_with('/')
              ? throw CodeGeneratorError(
                    "Preceding slashes are not allowed for folder name"
                )
              : folder
      ),
      at_(at),
      gt_(gt),
      fs_(fs),
      g_(g),
      add_json_generator_(add_json_generator),
      json_indents_(json_indents) {
    bool created = std::filesystem::create_directories(folder_);
    if (!created) {
        throw CodeGeneratorError("Could not create directory " + folder_);
    }
}

void CodeGenerator::Generate() {
    try {
        LexerGenerator lexer_generator(folder_, g_);
        lexer_generator.Generate();
    } catch (const LexerGeneratorError &e) {
        std::rethrow_exception(std::current_exception());
    }

    try {
        ParserGenerator parser_generator(
            folder_, g_, at_, gt_, fs_, add_json_generator_, json_indents_
        );
        parser_generator.Generate();
    } catch (const ParserGeneratorError &e) {
        std::rethrow_exception(std::current_exception());
    }
}

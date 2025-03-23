#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "BNFParser.h"
#include "TestHelpers.h"

TEST_CASE("Correct parsing of a simple grammar", "[BNFParser]") {
    std::string input = R"(
        id = [0-9]+
        <S> = <E>
        <E> = <E> '+' <T> | <T>
        <T> = <T> '*' <F> | <F>
        <F> = '(' <E> ')' | id
    )";
    GrammarParser gp(MakeStream(input));
    REQUIRE_NOTHROW(gp.Parse());
    REQUIRE(
        gp.Get().rules_.size() == 8
    );  // 7 grammar rules + 1 augmented rule S' = S
}

TEST_CASE("GrammarParser throws on empty grammar", "[BNFParserErrors]") {
    std::string input = R"()";
    GrammarParser gp(MakeStream(input));
    REQUIRE_THROWS_WITH(
        gp.Parse(), Catch::Matchers::ContainsSubstring("Empty grammar")
    );
}

TEST_CASE(
    "GrammarParser throws on quote terminal assignment", "[BNFParserErrors]"
) {
    std::string input = R"(
        'this wont work' = [0-9]+
    )";
    GrammarParser gp(MakeStream(input));
    REQUIRE_THROWS_WITH(
        gp.Parse(), Catch::Matchers::ContainsSubstring("Can't assign a regex")
    );
}

TEST_CASE("GrammarParser throws on empty production", "[BNFParserErrors]") {
    std::string input = R"(
        <S> = <A> | | <B>
    )";
    GrammarParser gp(MakeStream(input));
    REQUIRE_THROWS_WITH(
        gp.Parse(), Catch::Matchers::ContainsSubstring("Empty production")
    );
}

TEST_CASE(
    "GrammarParser throws on premature regex terminal usage",
    "[BNFParserErrors]"
) {
    std::string input = R"(
        <S> = '(' <E> ')'
        <E> = <E> '+' <T> | <T>
        <T> = <T> '*' <F> | <F>
        <F> = '(' <E> ')' | id
        id = [0-9]+
    )";
    GrammarParser gp(MakeStream(input));
    REQUIRE_THROWS_WITH(
        gp.Parse(),
        Catch::Matchers::ContainsSubstring("Unknown terminal encountered")
    );
}

TEST_CASE(
    "GrammarParser throws on incorrect usage of empty productions",
    "[BNFParserErrors]"
) {
    SECTION("Multiple empty productions in a rule") {
        std::string input = R"(
            <S> = <A> | EPSILON EPSILON | <B>
        )";
        GrammarParser gp(MakeStream(input));
        REQUIRE_THROWS_WITH(
            gp.Parse(),
            Catch::Matchers::ContainsSubstring("Epsilon can only be used in")
        );
    }

    SECTION("Stuff before empty production") {
        std::string input = R"(
            <S> = <A> | <C> EPSILON EPSILON | <B>
        )";
        GrammarParser gp(MakeStream(input));
        REQUIRE_THROWS_WITH(
            gp.Parse(),
            Catch::Matchers::ContainsSubstring("Epsilon can only be used in")
        );
    }

    SECTION("Stuff after empty production") {
        std::string input = R"(
            <S> = <A> | <C> EPSILON EPSILON | <B>
        )";
        GrammarParser gp(MakeStream(input));
        REQUIRE_THROWS_WITH(
            gp.Parse(),
            Catch::Matchers::ContainsSubstring("Epsilon can only be used in")
        );
    }

    SECTION("Stuff inbetween  empty productions") {
        std::string input = R"(
            <S> = <A> | EPSILON <C> EPSILON | <B>
        )";
        GrammarParser gp(MakeStream(input));
        REQUIRE_THROWS_WITH(
            gp.Parse(),
            Catch::Matchers::ContainsSubstring("Epsilon can only be used in")
        );
    }
}

TEST_CASE("GrammarParser throws on unterminated tokens", "[BNFParserErrors]") {
    SECTION("Unterminated quote terminal") {
        std::string input = R"(
            <S> = <A> | 'this is unterminated | <B>
        )";
        GrammarParser gp(MakeStream(input));
        REQUIRE_THROWS_WITH(
            gp.Parse(),
            Catch::Matchers::ContainsSubstring("Unterminated quote terminal")
        );
    }

    SECTION("Unterminated non-terminal") {
        std::string input = R"(
            <S> = <A> | <B | <C>
        )";
        GrammarParser gp(MakeStream(input));
        REQUIRE_THROWS_WITH(
            gp.Parse(), Catch::Matchers::ContainsSubstring("Unterminated `<`")
        );
    }
}

TEST_CASE(
    "GrammarParser throws an error on unexpected EOL/EOF", "[BNFParserErrors]"
) {
    SECTION("Unexpected EOL") {
        std::string input = R"(
            <C> = 
            <A> = <B>
        )";
        GrammarParser gp(MakeStream(input));
        REQUIRE_THROWS_WITH(
            gp.Parse(), Catch::Matchers::ContainsSubstring("Empty production")
        );
    }

    SECTION("Unexpected EOF") {
        std::string input = R"(
            <C> = 
        )";
        GrammarParser gp(MakeStream(input));
        REQUIRE_THROWS_WITH(
            gp.Parse(), Catch::Matchers::ContainsSubstring("Empty production")
        );
    }
}

// WARNING(helloclock): this test doesn't work yet
TEST_CASE(
    "GrammarParser throws on undefined non-terminals", "[BNFParserErrors]"
) {
    std::string input = R"(
        <S> = <A> | <B> | <C>
    )";
    GrammarParser gp(MakeStream(input));
    REQUIRE_THROWS_WITH(
        gp.Parse(), Catch::Matchers::ContainsSubstring("Unknown token")
    );
}

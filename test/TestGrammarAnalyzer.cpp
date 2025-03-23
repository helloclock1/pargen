#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>

#include "BNFParser.h"
#include "Entities.h"
#include "GrammarAnalyzer.h"
#include "Helpers.h"
#include "TestHelpers.h"

TEST_CASE("GrammarAnalyzer correctly computes sets 1", "[GrammarAnalyzer]") {
    std::string input = R"(
        int = [0-9]+
        <S> = <T> <E>
        <E> = '+' <T> <E> | EPSILON
        <T> = int
    )";

    GrammarParser gp(MakeStream(input));
    REQUIRE_NOTHROW(gp.Parse());

    Grammar g = gp.Get();
    GrammarAnalyzer ga(g);
    REQUIRE_NOTHROW(ga.ComputeFirst());
    REQUIRE_NOTHROW(ga.ComputeFollow());

    FirstSets first = ga.GetFirst();
    FollowSets follow = ga.GetFollow();

    SECTION("FIRST sets") {
        REQUIRE(
            first[Terminal{"int", " "}] ==
            std::set<Terminal>({Terminal{"int", " "}})
        );

        REQUIRE(
            first[NonTerminal{"S"}] ==
            std::set<Terminal>({Terminal{"int", " "}})
        );
        REQUIRE(
            first[NonTerminal{"E"}] ==
            std::set<Terminal>({EPSILON, Terminal{"+"}})
        );
        REQUIRE(
            first[NonTerminal{"T"}] ==
            std::set<Terminal>({Terminal{"int", " "}})
        );
    }

    SECTION("FIRST set for sequence") {
        REQUIRE(
            ga.FirstForSequence({NonTerminal{"T"}, NonTerminal{"E"}}) ==
            std::set<Terminal>({Terminal{"int", " "}})
        );
    }

    SECTION("FOLLOW sets") {
        REQUIRE(follow[NonTerminal{"S'"}] == std::set<Terminal>({T_EOF}));
        REQUIRE(follow[NonTerminal{"S"}] == std::set<Terminal>({T_EOF}));
        REQUIRE(follow[NonTerminal{"E"}] == std::set<Terminal>({T_EOF}));
        REQUIRE(
            follow[NonTerminal{"T"}] ==
            std::set<Terminal>({Terminal{"+"}, T_EOF})
        );
    }
}

TEST_CASE("GrammarAnalyzer correctly computes sets 2", "[GrammarAnalyzer]") {
    std::string input = R"(
        <E> = <E> '*' <B> | <E> '+' <B> | <B>
        <B> = '0' | '1'
    )";

    GrammarParser gp(MakeStream(input));
    REQUIRE_NOTHROW(gp.Parse());

    Grammar g = gp.Get();
    GrammarAnalyzer ga(g);
    ga.ComputeFirst();
    ga.ComputeFollow();

    FirstSets first = ga.GetFirst();
    FollowSets follow = ga.GetFollow();

    SECTION("FIRST sets") {
        for (const auto &s : {"*", "+", "0", "1"}) {
            REQUIRE(first[Terminal{s}] == std::set<Terminal>({Terminal{s}}));
        }

        REQUIRE(
            first[NonTerminal{"E"}] ==
            std::set<Terminal>({Terminal{"0"}, Terminal{"1"}})
        );
        REQUIRE(
            first[NonTerminal{"B"}] ==
            std::set<Terminal>({Terminal{"0"}, Terminal{"1"}})
        );
    }

    SECTION("FIRST for sequence") {
        REQUIRE(
            ga.FirstForSequence({NonTerminal{"E"}, NonTerminal{"B"}}) ==
            std::set<Terminal>({Terminal{"0"}, Terminal{"1"}})
        );
        REQUIRE(
            ga.FirstForSequence(
                {NonTerminal{"E"}, NonTerminal{"B"}, NonTerminal{"B"}}
            ) == std::set<Terminal>({Terminal{"0"}, Terminal{"1"}})
        );
        REQUIRE(
            ga.FirstForSequence({NonTerminal{"B"}, NonTerminal{"E"}}) ==
            std::set<Terminal>({Terminal{"0"}, Terminal{"1"}})
        );
    }

    SECTION("FOLLOW sets") {
        REQUIRE(
            follow[NonTerminal{"E"}] ==
            std::set<Terminal>({Terminal{"*"}, Terminal{"+"}, T_EOF})
        );
        REQUIRE(
            follow[NonTerminal{"B"}] ==
            std::set<Terminal>({Terminal{"*"}, Terminal{"+"}, T_EOF})
        );
    }
}

TEST_CASE(
    "GrammarAnalyzer correctly handles empty productions in FIRST and FOLLOW "
    "sets",
    "[GrammarAnalyzer]"
) {
    std::string input = R"(
        <S> = <A> 'b'
        <A> = 'a' <A> | EPSILON
    )";

    GrammarParser gp(MakeStream(input));
    REQUIRE_NOTHROW(gp.Parse());

    Grammar g = gp.Get();
    GrammarAnalyzer ga(g);
    ga.ComputeFirst();
    ga.ComputeFollow();

    FirstSets first = ga.GetFirst();
    FollowSets follow = ga.GetFollow();

    SECTION("FIRST sets") {
        REQUIRE(
            first[NonTerminal{"A"}] ==
            std::set<Terminal>({EPSILON, Terminal{"a"}})
        );
        REQUIRE(
            first[NonTerminal{"S"}] ==
            std::set<Terminal>({Terminal{"a"}, Terminal{"b"}})
        );
    }

    SECTION("FOLLOW sets") {
        REQUIRE(
            follow[NonTerminal{"A"}] == std::set<Terminal>({Terminal{"b"}})
        );
        REQUIRE(follow[NonTerminal{"S"}] == std::set<Terminal>({T_EOF}));
    }
}

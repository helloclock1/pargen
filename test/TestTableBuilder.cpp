#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>

#include "BNFParser.h"
#include "TableBuilder.h"
#include "TestHelpers.h"

TEST_CASE(
    "TableBuilder generates valid tables for simple grammar", "[TableBuilder]"
) {
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

    ParserTables tables(g, ga);
    REQUIRE_NOTHROW(tables.Generate());

    ActionTable action = tables.GetActionTable();
    GotoTable gotoTable = tables.GetGotoTable();

    REQUIRE_FALSE(action.empty());
    REQUIRE_FALSE(gotoTable.empty());
}

TEST_CASE("TableBuilder detects ambiguous grammar", "[TableBuilder]") {
    std::string input = R"(
        a = 'a'
        <S> = <A> | <B>
        <A> = a | EPSILON
        <B> = a | EPSILON
    )";

    GrammarParser gp(MakeStream(input));
    REQUIRE_NOTHROW(gp.Parse());

    Grammar g = gp.Get();
    GrammarAnalyzer ga(g);

    ParserTables tables(g, ga);
    REQUIRE_THROWS_AS(tables.Generate(), TableGeneratorError);
}

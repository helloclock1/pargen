#include <random>

#include "BNFParser.h"
#include "Entities.h"
#define CATCH_CONFIG_MAIN

#include <catch2/catch_test_macros.hpp>

#include "Automaton.h"
#include "Helpers.h"
#include "TestHelpers.h"

TEST_CASE("Automaton correctly computes closure", "[Automaton]") {
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
    ga.ComputeFirst();
    ga.ComputeFollow();

    FirstSets first = ga.GetFirst();
    FollowSets follow = ga.GetFollow();

    try {
        Automaton a(g, ga);

        REQUIRE(
            a.Closure({Automaton::State({Automaton::Item{0, 0, T_EOF}})}
            ).size() == g.rules_.size() - 1
        );  // initial state
        REQUIRE(
            a.Closure({Automaton::State({Automaton::Item{4, 1, T_EOF}})}
            ).size() == 1
        );  // finished terminal production `<T> = int .`

    } catch (const std::exception& e) {
        FAIL(e.what());
    }
}

TEST_CASE("Automaton correctly computes closure: stress", "[Automaton]") {
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
    ga.ComputeFirst();
    ga.ComputeFollow();

    FirstSets first = ga.GetFirst();
    FollowSets follow = ga.GetFollow();

    Automaton a(g, ga);

    std::mt19937 mt(time(0));
    for (size_t i = 0; i < 100; ++i) {
        Automaton::State state;
        std::set<size_t> used;

        for (size_t j = 0; j < mt() % (g.rules_.size() - 1) + 1; ++j) {
            size_t rule = mt() % (g.rules_.size() - 1) + 1;
            if (used.contains(rule)) {
                continue;
            }
            used.insert(rule);
            state.insert(Automaton::Item{
                rule, mt() % (g.rules_[rule].prod.size() + 1), T_EOF
            });
        }

        Automaton::State closure = a.Closure(state);

        for (const auto& item : state) {
            REQUIRE(closure.contains(item));
        }

        for (const auto& item : closure) {
            bool valid_item = false;
            if (state.contains(item)) {
                // item is definitely valid
                valid_item = true;
            } else {
                for (const auto& closure_item : closure) {
                    auto next_token = a.NextToken(closure_item);
                    if (next_token.has_value() &&
                        IsNonTerminal(next_token.value()) &&
                        std::get<NonTerminal>(next_token.value()) ==
                            g.rules_[item.rule_number_].lhs &&
                        item.dot_pos_ == 0) {
                        valid_item = true;
                        break;
                    }
                }
            }

            REQUIRE(valid_item);
        }
    }
}

TEST_CASE("Automaton correctly computes goto", "[Automaton]") {
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
    ga.ComputeFirst();
    ga.ComputeFollow();

    FirstSets first = ga.GetFirst();
    FollowSets follow = ga.GetFollow();

    Automaton a(g, ga);

    REQUIRE(
        a.Goto(
             Automaton::State{{Automaton::Item{0, 0, T_EOF}}}, NonTerminal{"S"}
        )
            .size() == 1
    );  // all input processed
    auto goto_state = a.Goto(
        Automaton::State{{Automaton::Item{4, 1, T_EOF}}}, Terminal{"int"}
    );
    REQUIRE(goto_state.size() == 0);  // nonexistent transition
}

/**
 * @file GrammarAnalyzer.h
 * @brief Provides a class for computing FIRST and FOLLOW sets for a given
 * grammar.
 * @author Vadim Melnikov
 * @version 1.0
 */
#pragma once

#include <set>
#include <vector>

#include "Entities.h"

/**
 * @class GrammarAnalyzer
 * @brief A class for computing FIRST and FOLLOW sets for a given grammar.
 */
class GrammarAnalyzer {
public:
    /**
     * @brief Constructs a GrammarAnalyzer object.
     * @param g The grammar to construct GrammarAnalyzer from and analyze.
     */
    explicit GrammarAnalyzer(const Grammar &g);

    /**
     * @brief Computes the FIRST sets for the grammar.
     */
    void ComputeFirst();
    /**
     * @brief Using precomputed FIRST sets, computes the FIRST set for a
     * sequence of tokens.
     * @param seq A sequence of tokens.
     * @return The FIRST set for the given sequence of tokens.
     */
    std::set<Terminal> FirstForSequence(const std::vector<Token> &seq) const;

    /**
     * @brief Computes the FOLLOW sets for the grammar.
     */
    void ComputeFollow();

    /**
     * @brief Returns the computed FIRST sets.
     * @return Const reference to the FIRST sets.
     */
    const FirstSets &GetFirst() const;
    /**
     * @brief Returns the computed FOLLOW sets.
     * @return Const reference to the FOLLOW sets.
     */
    const FollowSets &GetFollow() const;

private:
    const Grammar &g_;

    FirstSets first_;
    FollowSets follow_;
};
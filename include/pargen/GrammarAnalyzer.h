#pragma once

#include <set>
#include <vector>

#include "Entities.h"

class GrammarAnalyzer {
public:
    explicit GrammarAnalyzer(const Grammar &g);

    void ComputeFirst();
    std::set<Terminal> FirstForSequence(const std::vector<Token> &seq) const;

    void ComputeFollow();

    const FirstSets &GetFirst() const;
    const FollowSets &GetFollow() const;

private:
    const Grammar &g_;

    FirstSets first_;
    FollowSets follow_;
};
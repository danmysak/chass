#ifndef CHASS_BACKTRACKER_H
#define CHASS_BACKTRACKER_H

#include <vector>

#include "move.h"
#include "position.h"
#include "searcher.h"

class Backtracker : Searcher {
    int fullExaminationDepth, totalDepth;
    bool backtrack(const Position &position, std::vector<Move> &moves, std::vector<std::pair<int, int>> &progress);

public:
    using Searcher::Searcher;
    void search(const Position &position, int fullExaminationDepth, int totalDepth);
};

#endif // CHASS_BACKTRACKER_H
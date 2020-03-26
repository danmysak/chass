#ifndef CHASS_SEARCHER_H
#define CHASS_SEARCHER_H

#include "move.h"
#include "position.h"
#include "progressReporter.h"

class Searcher {
protected:
    void (*positionCallback)(const Position &, const std::vector<Move> &, int fullExaminationDepth);
    ProgressReporter &reporter;

public:
    Searcher(void (*positionCallback)(const Position &, const std::vector<Move> &, int fullExaminationDepth),
             ProgressReporter &reporter);
    ~Searcher();
};

#endif // CHASS_SEARCHER_H
#include "move.h"
#include "position.h"
#include "progressReporter.h"
#include "searcher.h"

Searcher::Searcher(void (*positionCallback)(const Position &, const std::vector<Move> &, int fullExaminationDepth),
                   ProgressReporter &reporter) : positionCallback(positionCallback), reporter(reporter) {
    reporter.start();
}

Searcher::~Searcher() {
    reporter.end();
}
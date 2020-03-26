#ifndef CHASS_MEETER_IN_THE_MIDDLE_H
#define CHASS_MEETER_IN_THE_MIDDLE_H

#include <vector>

#include "move.h"
#include "position.h"
#include "positionChain.h"
#include "searcher.h"

class MeeterInTheMiddle : Searcher {
    int depth;

    void iterate(PositionChain &chain,
                 void (*enumerate)(const Position &, std::vector<Move> &),
                 void (*perform)(Position &, const Move &), bool validate, int currentStage, int totalStages,
                 const Position *finalPosition = nullptr);
    static void traverse(const PositionChain &chain, int index, std::vector<Move> &reportedMoves);
    void merge(const PositionChain &frontChain, int frontIndex, const PositionChain &backChain, int backIndex);
    void consolidate(const PositionChain &frontChain, const PositionChain &backChain,
                     int currentStage, int totalStages);
    static double predictNextLevelSize(const PositionChain &chain);

public:
    using Searcher::Searcher;
    void search(const Position &position, int depth);
};

#endif // CHASS_MEETER_IN_THE_MIDDLE_H
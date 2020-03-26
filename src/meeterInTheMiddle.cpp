#include <algorithm>
#include <map>
#include <vector>

#include "advancer.h"
#include "analyzer.h"
#include "meeterInTheMiddle.h"
#include "move.h"
#include "position.h"
#include "positionChain.h"
#include "retractor.h"
#include "validator.h"

void MeeterInTheMiddle::iterate(PositionChain &chain,
                                void (*enumerate)(const Position &, std::vector<Move> &),
                                void (*perform)(Position &, const Move &), bool validate,
                                int currentStage, int totalStages, const Position *finalPosition) {
    chain.startNextLevel();
    const auto &last = chain.secondLastLevel();
    auto &next = chain.lastLevel();
    for (int i = 0; i < last.length; ++i) {
        int index = i + last.startingIndex;
        reporter.reportProgress({{currentStage, totalStages}, {i, last.length}});
        std::vector<Move> moves;
        Position position = Position(chain.get(index).position);
        enumerate(position, moves);
        for (const auto &move : moves) {
            if (finalPosition != nullptr && move.piece == Pawn
                && ((move.side == White && move.startingSquare.rank == 1)
                    || (move.side == Black && move.startingSquare.rank == 6))
                && finalPosition->isPieceInSquare(move.startingSquare, move.side, move.piece)) {
                continue;
            }
            Position nextPosition = position;
            perform(nextPosition, move);
            if ((validate && Validator::validate(nextPosition))
                || (!validate && Validator::validateChecks(nextPosition))) {
                chain.add(nextPosition.pack(), move, index);
            }
        }
    }
}

void MeeterInTheMiddle::traverse(const PositionChain &chain, int index, std::vector<Move> &reportedMoves) {
    for (int level = chain.levelCount() - 1; level > 0; --level) {
        auto &current = chain.get(index);
        reportedMoves.emplace_back(current.move);
        index = current.nextInChain;
    }
}

void MeeterInTheMiddle::merge(const PositionChain &frontChain, int frontIndex,
                              const PositionChain &backChain, int backIndex) {
    std::vector<Move> reportedMoves;
    reportedMoves.reserve(depth);
    traverse(backChain, backIndex, reportedMoves);
    std::reverse(reportedMoves.begin(), reportedMoves.end());
    traverse(frontChain, frontIndex, reportedMoves);
    positionCallback(Analyzer::getStartingPosition(), reportedMoves, depth);
}

void MeeterInTheMiddle::consolidate(const PositionChain &frontChain, const PositionChain &backChain,
                                    int currentStage, int totalStages) {
    const auto &frontLevel = frontChain.lastLevel();
    const auto &backLevel = backChain.lastLevel();
    int totalSteps = frontLevel.length + backLevel.length;
    int currentStep = 0;
    std::map<std::string, std::vector<int>> positionMap;
    std::vector<std::pair<const PositionChain*, const PositionChainLevel*>> chains;
    int frontThenBack = frontLevel.length < backLevel.length;
    if (frontThenBack) {
        chains = {{&frontChain, &frontLevel}, {&backChain, &backLevel}};
    } else {
        chains = {{&backChain, &backLevel}, {&frontChain, &frontLevel}};
    }
    for (int stage = 0; stage < 2; ++stage) {
        int maxIndex = chains[stage].second->startingIndex + chains[stage].second->length;
        for (int index = chains[stage].second->startingIndex; index < maxIndex; ++index) {
            reporter.reportProgress({{currentStage, totalStages}, {currentStep, totalSteps}});
            std::string FEN = Position(chains[stage].first->get(index).position).toFENPlacement();
            auto occurrence = positionMap.find(FEN);
            if (stage == 0) {
                if (occurrence == positionMap.end()) {
                    positionMap[FEN] = {index};
                } else {
                    occurrence->second.emplace_back(index);
                }
            } else {
                if (occurrence != positionMap.end()) {
                    for (auto &anotherIndex : occurrence->second) {
                        int frontIndex = frontThenBack ? anotherIndex : index;
                        int backIndex = frontThenBack ? index : anotherIndex;
                        if (Position(frontChain.get(frontIndex).position).canBeSpecializationOf(
                                Position(backChain.get(backIndex).position))) {
                            merge(frontChain, frontIndex, backChain, backIndex);
                        }
                    }
                }
            }
            ++currentStep;
        }
    }
}

double MeeterInTheMiddle::predictNextLevelSize(const PositionChain &chain) {
    if (chain.levelCount() < 2) {
        return 1.0;
    }
    auto lastLevelSize = static_cast<double>(chain.lastLevel().length);
    return lastLevelSize * lastLevelSize / chain.secondLastLevel().length;
}

void MeeterInTheMiddle::search(const Position &position, int depth) {
    this->depth = depth;
    PositionChain frontChain, backChain;
    frontChain.add(Analyzer::getStartingPosition().pack(), Move(), -1);
    if (Validator::validate(position)) {
        backChain.add(position.pack(), Move(), -1);
    }
    int totalStages = depth + 1; // +1 is the consolidation
    int iteration = 0;
    while (iteration < depth) {
        if (backChain.lastLevel().length == 0) {
            return;
        }
        if (predictNextLevelSize(backChain) < predictNextLevelSize(frontChain)) { // Retracting
            iterate(backChain, Retractor::enumerateMoves, Retractor::retract, true, iteration, totalStages);
        } else { // Advancing
            iterate(frontChain, Advancer::enumerateMoves, Advancer::advance, false, iteration, totalStages, &position);
        }
        ++iteration;
    }
    consolidate(frontChain, backChain, totalStages - 1, totalStages);
}
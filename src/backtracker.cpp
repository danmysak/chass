#include <vector>

#include "analyzer.h"
#include "backtracker.h"
#include "move.h"
#include "position.h"
#include "retractor.h"
#include "validator.h"

bool Backtracker::backtrack(const Position &position, std::vector<Move> &moves,
                            std::vector<std::pair<int, int>> &progress) {
    if (!Validator::validate(position)) {
        return false;
    }

    int currentDepth = moves.size();
    bool fullExamination = currentDepth < fullExaminationDepth;
    bool atDeepest = currentDepth == totalDepth;

    if (atDeepest || Analyzer::canBeStarting(position)) {
        positionCallback(position, moves, fullExaminationDepth);
        if (atDeepest || !fullExamination) {
            return true;
        }
    }

    bool found = false;
    std::vector<Move> retractMoves;
    Retractor::enumerateMoves(position, retractMoves);
    progress.emplace_back(std::make_pair(0, retractMoves.size()));
    for (const auto &retractMove : retractMoves) {
        reporter.reportProgress(progress);
        Position previous = position;
        Retractor::retract(previous, retractMove);
        moves.emplace_back(retractMove);
        if (backtrack(previous, moves, progress)) {
            found = true;
        }
        moves.pop_back();
        if (found && !fullExamination) {
            progress.pop_back();
            return true;
        }
        ++progress.back().first;
    }
    progress.pop_back();
    return found;
}

void Backtracker::search(const Position &position, int fullExaminationDepth, int totalDepth) {
    this->fullExaminationDepth = fullExaminationDepth;
    this->totalDepth = totalDepth;
    std::vector<Move> moves = {};
    std::vector<std::pair<int, int>> progress = {};
    backtrack(position, moves, progress);
}
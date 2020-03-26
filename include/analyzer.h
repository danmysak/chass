#ifndef CHASS_ANALYZER_H
#define CHASS_ANALYZER_H

#include <string>
#include <vector>

#include "enums.h"
#include "move.h"
#include "piece.h"
#include "position.h"
#include "square.h"

class Analyzer {
    static bool isRangeEmpty(const Position &position, Square square, int fileStep, int rankStep, int max);
    static bool isAttackingAsRook(const Position &position, const Piece &piece, int fileDelta, int rankDelta);
    static bool isAttackingAsBishop(const Position &position, const Piece &piece, int fileDelta, int rankDelta);
    static bool isAttacking(const Position &position, const Piece &piece, const Square &square);

public:
    static void getLegalMoves(const Position &position, std::vector<Move> &moves, bool returnJustFirst = false);
    static bool isUnderAttack(const Position &position, Sides side, const Square &square);
    static bool isInCheck(const Position &position, Sides side);
    static bool isInCheck(const Position &position);
    static bool isCheckmated(const Position &position);
    static bool isInCastlingPosition(const Position &position, Sides side, CastlingSides castlingSide,
                                     bool needBreakingMove = false, const Move *move = nullptr);
    static Position getStartingPosition();
    static bool canBeStarting(const Position &position);
    static Move interpretShortAlgebraic(const std::string &notation, const Position &position, bool &check, bool &mate);
};

#endif // CHASS_ANALYZER_H
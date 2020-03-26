#ifndef CHASS_VALIDATOR_H
#define CHASS_VALIDATOR_H

#include <vector>

#include "piece.h"
#include "pieceCounts.h"
#include "position.h"

class Validator {
    static void validateUserKings(const std::vector<Piece> &pieces);
    static void validateUserCounts(const PieceCounts &counts);
    static void validateUserPawns(const std::vector<Piece> &pieces);
    static void validateUserEnPassant(const Position &position);
    static void validateUserHalfMoves(const Position &position);
    static void validateUserFullMoves(const Position &position);
    static bool validateCounts(const PieceCounts &counts);
    static bool validateRequiredMoveNumber(const Position &position, Sides side);
    static bool validateInitial(const Position &position);

public:
    static bool validateChecks(const Position &position);
    static bool validate(const Position &position);
    static std::pair<bool, std::string> validateAndStrictenUserPosition(Position &position);
};

#endif // CHASS_VALIDATOR_H
#ifndef CHASS_MOVE_H
#define CHASS_MOVE_H

#include <string>

#include "enums.h"
#include "square.h"

class Move {
    static std::string addSuffix(const std::string &move, bool check, bool mate);
    static std::string getCastlingNotation(MoveTypes type);

public:
    Pieces piece;
    Sides side;
    MoveTypes type;
    Square startingSquare;
    Square targetSquare;
    Pieces capturedPiece;
    Pieces promotedPiece;

    bool sameAs(const Move& move);
    static bool parseCastlingNotation(const std::string &notation, MoveTypes &type);
    [[nodiscard]] std::string toLongAlgebraic(bool check = false, bool mate = false) const;
    Move(Pieces piece, Sides side, MoveTypes type, const Square &startingSquare, const Square &targetSquare,
        Pieces capturedPiece = King, Pieces promotedPiece = King);
    Move();
};

#endif // CHASS_MOVE_H
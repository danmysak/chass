#ifndef CHASS_PIECE_H
#define CHASS_PIECE_H

#include <string>

#include "enums.h"
#include "square.h"

struct Piece {
    Pieces kind;
    Sides side;
    Square square;

    [[nodiscard]] char toFEN() const;
    static std::string toAlgebraic(Pieces kind);
    static Pieces fromAlgebraic(char c);
    [[nodiscard]] bool isAdjacent(const Piece &to) const;
    explicit Piece(Pieces kind = King, Sides side = White, const Square &square = Square());
};

#endif // CHASS_PIECE_H
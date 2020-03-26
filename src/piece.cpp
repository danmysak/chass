#include <string>

#include "enums.h"
#include "exceptions.h"
#include "piece.h"
#include "square.h"

[[nodiscard]] char Piece::toFEN() const {
    switch (kind) {
        case Pawn:
            return side == White ? 'P' : 'p';
        case Knight:
            return side == White ? 'N' : 'n';
        case Bishop:
            return side == White ? 'B' : 'b';
        case Rook:
            return side == White ? 'R' : 'r';
        case Queen:
            return side == White ? 'Q' : 'q';
        case King:
            return side == White ? 'K' : 'k';
    }
    return '-'; // Avoiding the no-return warning
}

std::string Piece::toAlgebraic(Pieces kind) {
    switch (kind) {
        case Knight:
            return "N";
        case Bishop:
            return "B";
        case Rook:
            return "R";
        case Queen:
            return "Q";
        case King:
            return "K";
        default:
            return "";
    }
}

Pieces Piece::fromAlgebraic(char c) {
    switch (c) {
        case 'N':
            return Knight;
        case 'B':
            return Bishop;
        case 'R':
            return Rook;
        case 'Q':
            return Queen;
        case 'K':
            return King;
        default:
            throw UnknownPiece(c);
    }
}

[[nodiscard]] bool Piece::isAdjacent(const Piece &to) const {
    return abs(square.file - to.square.file) <= 1 && abs(square.rank - to.square.rank) <= 1;
}

Piece::Piece(Pieces kind, Sides side, const Square &square) : kind(kind), side(side), square(square) {}
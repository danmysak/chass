#ifndef CHASS_ADVANCER_H
#define CHASS_ADVANCER_H

#include <vector>

#include "enums.h"
#include "move.h"
#include "piece.h"
#include "position.h"

class Advancer {
    static void updatePieces(Position &position, const Move &move);
    static void updateCastling(Position &position, const Move &move);
    static void updateEnPassant(Position &position, const Move &move);
    static void updateMoves(Position &position, const Move &move);
    static Move constructMove(const Piece &piece, MoveTypes type, const Square &targetSquare,
                              Pieces capturedPiece = King);
    static void enumerateKingMoves(const Position &position, const Piece &piece, std::vector<Move> &moves);
    static void enumerateLinearMoves(const Position &position, const Piece &piece, int fileDirection, int rankDirection,
                                     std::vector<Move> &moves);
    static void enumerateRookLikeMoves(const Position &position, const Piece &piece, std::vector<Move> &moves);
    static void enumerateBishopLikeMoves(const Position &position, const Piece &piece, std::vector<Move> &moves);
    static void enumerateKnightMoves(const Position &position, const Piece &piece, std::vector<Move> &moves);
    static void appendPromotionMove(const Move &move, Pieces promotedPiece, std::vector<Move> &moves);
    static void enumeratePromotionMoves(const Move &move, std::vector<Move> &moves);
    static void enumeratePawnMoves(const Position &position, const Piece &piece, std::vector<Move> &moves);

public:
    static void enumerateMoves(const Position &position, std::vector<Move> &moves);
    static void advance(Position &position, const Move &move);
};

#endif // CHASS_ADVANCER_H
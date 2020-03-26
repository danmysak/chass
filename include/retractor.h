#ifndef CHASS_RETRACTOR_H
#define CHASS_RETRACTOR_H

#include <vector>

#include "enums.h"
#include "move.h"
#include "piece.h"
#include "position.h"
#include "square.h"

class Retractor {
    static void updatePieces(Position &position, const Move &move);
    static void updateCastling(Position &position, const Move &move);
    static void updateEnPassant(Position &position, const Move &move);
    static void updateMoves(Position &position, const Move &move);
    static Move constructMove(const Piece &piece, MoveTypes type, const Square &startingSquare,
                              Pieces capturedPiece = King);
    static void appendCaptureMove(const Move &move, Pieces capturedPiece, std::vector<Move> &moves);
    static void enumerateCaptureMoves(const Move &move, std::vector<Move> &moves);
    static void enumeratePotentialCaptureMoves(const Piece &piece, const Square &square, Ternary pawnOrCapture,
                                               std::vector<Move> &moves);
    static void enumerateKingMoves(const Position &position, const Piece &piece, Ternary pawnOrCapture,
                                   std::vector<Move> &moves);
    static void enumerateLinearMoves(const Position &position, const Piece &piece, int fileDirection, int rankDirection,
                                     Ternary pawnOrCapture, std::vector<Move> &moves);
    static void enumerateRookLikeMoves(const Position &position, const Piece &piece, Ternary pawnOrCapture,
                                       std::vector<Move> &moves);
    static void enumerateBishopLikeMoves(const Position &position, const Piece &piece, Ternary pawnOrCapture,
                                         std::vector<Move> &moves);
    static void enumerateKnightMoves(const Position &position, const Piece &piece, Ternary pawnOrCapture,
                                     std::vector<Move> &moves);
    static void enumeratePawnMoves(const Position &position, const Piece &piece, Ternary enPassant,
                                   std::vector<Move> &moves);
    static void enumeratePromotionMoves(const Position &position, const Piece &piece, std::vector<Move> &moves);

public:
    static void enumerateMoves(const Position &position, std::vector<Move> &moves);
    static void retract(Position &position, const Move &move);
};

#endif // CHASS_RETRACTOR_H
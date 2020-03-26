#ifndef CHASS_POSITION_H
#define CHASS_POSITION_H

#include <bitset>
#include <string>
#include <vector>

#include "enums.h"
#include "extraInfo.h"
#include "piece.h"
#include "pieceCounts.h"
#include "squareInfo.h"

using PackedPosition = std::bitset<1 + 64 * 4 + 4 * 2 + (2 + 3) + (1 + 16) + (1 + 16)>;
// turn + pieces on board + castling + en passant + half moves + full moves

class Position {
    std::vector<Piece> whitePieces, blackPieces;
    PieceCounts whiteCounts = {}, blackCounts = {};
    SquareInfo squares[8][8];
    Sides turn = White;
    ExtraInfo extraInfo = ExtraInfo();
    bool halfMoveLog = false, fullMoveLog = false;
    int halfMoves = 0, fullMoves = 0;

    static void updateCounts(PieceCounts &counts, const Piece &piece, bool increment = true);
    void swapPieces(std::vector<Piece> &pieces, int indexA, int indexB);
    void writeToPacked(PackedPosition &packed, int &position, int value, int bits) const;
    void writeTernaryToPacked(PackedPosition &packed, int &position, Ternary value) const;
    void writeBoolToPacked(PackedPosition &packed, int &position, bool value) const;
    int readFromPacked(const PackedPosition &packed, int &position, int bits) const;
    Ternary readTernaryFromPacked(const PackedPosition &packed, int &position) const;
    bool readBoolFromPacked(const PackedPosition &packed, int &position) const;
    void init();

public:
    [[nodiscard]] bool canBeSpecializationOf(const Position &position) const;
    void movePiece(const Square &current, const Square &another);
    void addPiece(const Square &square, Pieces kind, Sides side);
    void removePiece(const Square &square);
    void setTurn(Sides side);
    [[nodiscard]] Sides getTurn() const;
    void setCastling(Sides side, CastlingSides castlingSide, Ternary state);
    [[nodiscard]] Ternary getCastling(Sides side, CastlingSides castlingSide) const;
    void setEnPassant(Ternary state, int file = 0);
    [[nodiscard]] Ternary getEnPassant() const;
    [[nodiscard]] int getEnPassantFile() const;
    void setHalfMoves(bool log, int counter = 0);
    void incrementHalfMoves();
    void decrementHalfMoves();
    [[nodiscard]] bool getHalfMoveLog() const;
    [[nodiscard]] int getHalfMoveCounter() const;
    void setFullMoves(bool log, int counter = 0);
    void incrementFullMoves();
    void decrementFullMoves();
    [[nodiscard]] bool getFullMoveLog() const;
    [[nodiscard]] int getFullMoveCounter() const;
    [[nodiscard]] int getPlyCounter() const;
    [[nodiscard]] int getCompletedMoves(Sides side) const;
    [[nodiscard]] const std::vector<Piece> &getPieces(Sides side) const;
    [[nodiscard]] const Piece &getKing(Sides side) const;
    [[nodiscard]] const PieceCounts &getPieceCounts(Sides side) const;
    [[nodiscard]] bool isOnBoard(const Square &square) const;
    [[nodiscard]] const SquareInfo &getSquareInfo(const Square &square) const;
    [[nodiscard]] const Piece &getPiece(const SquareInfo &square) const;
    [[nodiscard]] bool isSquareEmpty(const Square &square) const;
    [[nodiscard]] bool isPieceInSquare(const Square &square, Sides side, Pieces kind) const;
    [[nodiscard]] std::string toFENPlacement(bool includeTurn = false) const;
    [[nodiscard]] PackedPosition pack() const;
    explicit Position(const PackedPosition &packed);
    Position();
};

#endif // CHASS_POSITION_H
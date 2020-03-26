#include <string>
#include <vector>

#include "enums.h"
#include "exceptions.h"
#include "helper.h"
#include "piece.h"
#include "pieceCounts.h"
#include "position.h"
#include "squareInfo.h"

void Position::updateCounts(PieceCounts &counts, const Piece &piece, bool increment) {
    int delta = increment ? 1 : -1;
    switch (piece.kind) {
        case Queen:
            counts.queen += delta;
            break;
        case Rook:
            counts.rook += delta;
            break;
        case Bishop:
            if ((piece.square.file + piece.square.rank) % 2 == 0) {
                counts.blackSquareBishop += delta;
            } else {
                counts.whiteSquareBishop += delta;
            }
            break;
        case Knight:
            counts.knight += delta;
            break;
        case Pawn:
            counts.pawn += delta;
            break;
        default:
            break;
    }
}

void Position::swapPieces(std::vector<Piece> &pieces, int indexA, int indexB) {
    Square a = pieces[indexA].square;
    Square b = pieces[indexB].square;
    squares[a.file][a.rank].index = indexB;
    squares[b.file][b.rank].index = indexA;
    std::swap(pieces[indexA], pieces[indexB]);
}

[[nodiscard]] bool Position::canBeSpecializationOf(const Position &position) const {
    return position.turn == turn && position.toFENPlacement() == toFENPlacement()
           && Helper::canBeSpecialization(extraInfo.whiteKingCastling, position.extraInfo.whiteKingCastling)
           && Helper::canBeSpecialization(extraInfo.whiteQueenCastling, position.extraInfo.whiteQueenCastling)
           && Helper::canBeSpecialization(extraInfo.blackKingCastling, position.extraInfo.blackKingCastling)
           && Helper::canBeSpecialization(extraInfo.blackQueenCastling, position.extraInfo.blackQueenCastling)
           && Helper::canBeSpecialization(extraInfo.enPassant, position.extraInfo.enPassant)
           && !(extraInfo.enPassant == True && position.extraInfo.enPassant == True
                && extraInfo.enPassantFile != position.extraInfo.enPassantFile)
           && Helper::canBeSpecialization(halfMoveLog, halfMoves, position.halfMoveLog, position.halfMoves)
           && Helper::canBeSpecialization(fullMoveLog, fullMoves, position.fullMoveLog, position.fullMoves);
}

void Position::movePiece(const Square &current, const Square &another) {
    SquareInfo &currentInfo = squares[current.file][current.rank];
    std::vector<Piece> &pieces = currentInfo.side == White ? whitePieces : blackPieces;
    pieces[currentInfo.index].square = another;
    squares[another.file][another.rank] = currentInfo;
    currentInfo.occupied = false;
}

void Position::addPiece(const Square &square, Pieces kind, Sides side) {
    std::vector<Piece> &pieces = side == White ? whitePieces : blackPieces;
    PieceCounts &counts = side == White ? whiteCounts : blackCounts;
    squares[square.file][square.rank] = SquareInfo(true, side, pieces.size());
    pieces.emplace_back(Piece(kind, side, square));
    updateCounts(counts, pieces.back());
}

void Position::removePiece(const Square &square) {
    SquareInfo &squareInfo = squares[square.file][square.rank];
    std::vector<Piece> &pieces = squareInfo.side == White ? whitePieces : blackPieces;
    PieceCounts &counts = squareInfo.side == White ? whiteCounts : blackCounts;
    swapPieces(pieces, squareInfo.index, static_cast<int>(pieces.size()) - 1);
    squares[square.file][square.rank].occupied = false;
    updateCounts(counts, pieces.back(), false);
    pieces.pop_back();
}

void Position::setTurn(Sides side) {
    turn = side;
}

[[nodiscard]] Sides Position::getTurn() const {
    return turn;
}

void Position::setCastling(Sides side, CastlingSides castlingSide, Ternary state) {
    if (side == White) {
        if (castlingSide == Kingside) {
            extraInfo.whiteKingCastling = state;
        } else {
            extraInfo.whiteQueenCastling = state;
        }
    } else {
        if (castlingSide == Kingside) {
            extraInfo.blackKingCastling = state;
        } else {
            extraInfo.blackQueenCastling = state;
        }
    }
}

[[nodiscard]] Ternary Position::getCastling(Sides side, CastlingSides castlingSide) const {
    if (side == White) {
        if (castlingSide == Kingside) {
            return extraInfo.whiteKingCastling;
        } else {
            return extraInfo.whiteQueenCastling;
        }
    } else {
        if (castlingSide == Kingside) {
            return extraInfo.blackKingCastling;
        } else {
            return extraInfo.blackQueenCastling;
        }
    }
}

void Position::setEnPassant(Ternary state, int file) {
    extraInfo.enPassant = state;
    extraInfo.enPassantFile = file;
}

[[nodiscard]] Ternary Position::getEnPassant() const {
    return extraInfo.enPassant;
}

[[nodiscard]] int Position::getEnPassantFile() const {
    return extraInfo.enPassantFile;
}

void Position::setHalfMoves(bool log, int counter) {
    halfMoveLog = log;
    halfMoves = counter;
}

void Position::incrementHalfMoves() {
    ++halfMoves;
}

void Position::decrementHalfMoves() {
    --halfMoves;
}

[[nodiscard]] bool Position::getHalfMoveLog() const {
    return halfMoveLog;
}

[[nodiscard]] int Position::getHalfMoveCounter() const {
    return halfMoves;
}

void Position::setFullMoves(bool log, int counter) {
    fullMoveLog = log;
    fullMoves = counter;
}

void Position::incrementFullMoves() {
    ++fullMoves;
}

void Position::decrementFullMoves() {
    --fullMoves;
}

[[nodiscard]] bool Position::getFullMoveLog() const {
    return fullMoveLog;
}

[[nodiscard]] int Position::getFullMoveCounter() const {
    return fullMoves;
}

[[nodiscard]] int Position::getPlyCounter() const {
    return fullMoveLog ? fullMoves * 2 - (turn == White ? 1 : 0) : 0;
}

[[nodiscard]] int Position::getCompletedMoves(Sides side) const {
    return fullMoveLog ? (turn == Black && side == White ? fullMoves : fullMoves - 1) : -1;
}

[[nodiscard]] const std::vector<Piece> &Position::getPieces(Sides side) const {
    return side == White ? whitePieces : blackPieces;
}

[[nodiscard]] const Piece &Position::getKing(Sides side) const {
    for (auto &piece : getPieces(side)) {
        if (piece.kind == King) {
            return piece;
        }
    }
    throw NoKings();
}

[[nodiscard]] const PieceCounts &Position::getPieceCounts(Sides side) const {
    return side == White ? whiteCounts : blackCounts;
}

[[nodiscard]] bool Position::isOnBoard(const Square &square) const {
    return square.file >= 0 && square.rank >= 0 && square.file < 8 && square.rank < 8;
}

[[nodiscard]] const SquareInfo &Position::getSquareInfo(const Square &square) const {
    return squares[square.file][square.rank];
}

[[nodiscard]] const Piece &Position::getPiece(const SquareInfo &square) const {
    return square.side == White ? whitePieces[square.index] : blackPieces[square.index];
}

[[nodiscard]] bool Position::isSquareEmpty(const Square &square) const {
    return !getSquareInfo(square).occupied;
}

[[nodiscard]] bool Position::isPieceInSquare(const Square &square, Sides side, Pieces kind) const {
    const SquareInfo &info = getSquareInfo(square);
    return info.occupied && info.side == side && getPieces(side)[info.index].kind == kind;
}

[[nodiscard]] std::string Position::toFENPlacement(bool includeTurn) const {
    std::string result;
    for (int rank = 7; rank >= 0; --rank) {
        int emptyCount = 0;
        for (auto &file : squares) {
            if (file[rank].occupied) {
                if (emptyCount > 0) {
                    result += std::to_string(emptyCount);
                    emptyCount = 0;
                }
                result += getPiece(file[rank]).toFEN();
            } else {
                ++emptyCount;
            }
        }
        if (emptyCount > 0) {
            result += std::to_string(emptyCount);
        }
        if (rank > 0) {
            result += '/';
        }
    }
    if (includeTurn) {
        result += ' ';
        result += turn == White ? 'w' : 'b';
    }
    return result;
}

void Position::init() {
    whitePieces.reserve(17); // One extra piece for invalid retractions (they are to be discarded later)
    blackPieces.reserve(17);
    for (auto &file : squares) {
        for (auto &square : file) {
            square.occupied = false;
        }
    }
}

void Position::writeToPacked(PackedPosition &packed, int &position, int value, int bits) const {
    for (int bit = bits - 1; bit >= 0; --bit) {
        packed[position] = (value >> bit) & 1;
        ++position;
    }
}

void Position::writeTernaryToPacked(PackedPosition &packed, int &position, Ternary value) const {
    writeToPacked(packed, position, value == Unknown ? 0b11 : (value == False ? 0b00 : 0b01), 2);
}

void Position::writeBoolToPacked(PackedPosition &packed, int &position, bool value) const {
    writeToPacked(packed, position, value ? 1 : 0, 1);
}

int Position::readFromPacked(const PackedPosition &packed, int &position, int bits) const {
    int value = 0;
    for (int i = 0; i < bits; ++i) {
        value = (value << 1) + (packed[position] ? 1 : 0);
        ++position;
    }
    return value;
}

Ternary Position::readTernaryFromPacked(const PackedPosition &packed, int &position) const {
    int value = readFromPacked(packed, position, 2);
    return value == 0b11 ? Unknown : (value == 0b00 ? False : True);
}

bool Position::readBoolFromPacked(const PackedPosition &packed, int &position) const {
    return readFromPacked(packed, position, 1) == 1;
}

PackedPosition Position::pack() const {
    PackedPosition packed;
    int p = 0;

    writeToPacked(packed, p, static_cast<int>(turn), 1);
    for (int file = 0; file < 8; ++file) {
        for (int rank = 0; rank < 8; ++rank) {
            const auto &square = squares[file][rank];
            int value;
            if (square.occupied) {
                value = (static_cast<int>(square.side) << 3) + static_cast<int>(square.side == White
                        ? whitePieces[square.index].kind : blackPieces[square.index].kind);
            } else {
                value = 0b1111;
            }
            writeToPacked(packed, p, value, 4);
        }
    }

    writeTernaryToPacked(packed, p, extraInfo.whiteKingCastling);
    writeTernaryToPacked(packed, p, extraInfo.whiteQueenCastling);
    writeTernaryToPacked(packed, p, extraInfo.blackKingCastling);
    writeTernaryToPacked(packed, p, extraInfo.blackQueenCastling);
    writeTernaryToPacked(packed, p, extraInfo.enPassant);
    writeToPacked(packed, p, extraInfo.enPassantFile, 3);

    writeBoolToPacked(packed, p, halfMoveLog);
    writeToPacked(packed, p, halfMoves, 16);
    writeBoolToPacked(packed, p, fullMoveLog);
    writeToPacked(packed, p, fullMoves, 16);

    return packed;
}

Position::Position(const PackedPosition &packed) {
    init();
    int p = 0;

    turn = static_cast<Sides>(readFromPacked(packed, p, 1));
    for (int file = 0; file < 8; ++file) {
        for (int rank = 0; rank < 8; ++rank) {
            int value = readFromPacked(packed, p, 4);
            if (value != 0b1111) {
                addPiece(Square(file, rank), static_cast<Pieces>(value & 0b111), static_cast<Sides>(value >> 3));
            }
        }
    }

    extraInfo.whiteKingCastling = readTernaryFromPacked(packed, p);
    extraInfo.whiteQueenCastling = readTernaryFromPacked(packed, p);
    extraInfo.blackKingCastling = readTernaryFromPacked(packed, p);
    extraInfo.blackQueenCastling = readTernaryFromPacked(packed, p);
    extraInfo.enPassant = readTernaryFromPacked(packed, p);
    extraInfo.enPassantFile = readFromPacked(packed, p, 3);

    halfMoveLog = readBoolFromPacked(packed, p);
    halfMoves = readFromPacked(packed, p, 16);
    fullMoveLog = readBoolFromPacked(packed, p);
    fullMoves = readFromPacked(packed, p, 16);
}

Position::Position() {
    init();
}
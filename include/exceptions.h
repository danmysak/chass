#ifndef CHASS_EXCEPTIONS_H
#define CHASS_EXCEPTIONS_H

#include <exception>
#include <string>
#include <utility>

#include "enums.h"
#include "helper.h"

class UnknownPiece: public std::exception {
    std::string msg;
public:
    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }
    explicit UnknownPiece(char c) : msg("Unknown piece: '" + Helper::charToString(c) + "'") {}
};

class UnknownFile: public std::exception {
    std::string msg;
public:
    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }
    explicit UnknownFile(char c) : msg("Unknown file: '" + Helper::charToString(c) + "'") {}
};

class UnknownRank: public std::exception {
    std::string msg;
public:
    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }
    explicit UnknownRank(char c) : msg("Unknown rank: '" + Helper::charToString(c) + "'") {}
};

class NoKings: public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "No kings assigned to the side";
    }
};

class ManyKings: public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "More than one king assigned to the side";
    }
};

class TooManyPieces: public std::exception {
    [[nodiscard]] const char* what() const noexcept override {
        return "More than 16 pieces assigned to the side";
    }
};

class TooManyPawns: public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "More than 8 pawns assigned to the side";
    }
};

class InvalidPieceCount: public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "Piece count is not possible in a legal game";
    }
};

class OutlierPawn: public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "Pawn placed on a boundary rank";
    }
};

class InvalidEnPassant: public std::exception {
    std::string msg;
public:
    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }
    explicit InvalidEnPassant(int file) : msg("En passant at file '" + Helper::charToString('a' + file) +
                                              "' is incorrectly defined as possible") {}
};

class HalfMovesEnPassantViolation: public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "The half-move clock cannot be positive when an en passant capture is possible";
    }
};

class NegativeFullMoves: public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "The full-move number should be positive";
    }
};

class HalfMovesExceedFullMoves: public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "The half-move clock is exceeding (twice) the full-move counter";
    }
};

class InvalidStartingPosition: public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "Position is incorrectly defined as starting";
    }
};

class FENParseError: public std::exception {
    std::string msg;
public:
    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }
    explicit FENParseError(std::string msg) : msg(move(msg)) {}
};

class AlgebraicInterpretationError: public std::exception {
    std::string msg;
public:
    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }
    explicit AlgebraicInterpretationError(std::string msg) : msg(move(msg)) {}
};

#endif // CHASS_EXCEPTIONS_H
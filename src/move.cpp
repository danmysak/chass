#include <string>

#include "enums.h"
#include "move.h"
#include "piece.h"
#include "square.h"

std::string Move::addSuffix(const std::string &move, bool check, bool mate) {
    if (mate) {
        return move + "#";
    } else if (check) {
        return move + "+";
    } else {
        return move;
    }
}

std::string Move::getCastlingNotation(MoveTypes type) {
    switch (type) {
        case KingsideCastling:
            return "0-0";
        case QueensideCastling:
            return "0-0-0";
        default:
            return "";
    }
}

bool Move::sameAs(const Move& move) {
    return move.side == side && move.type == type
           && (type == KingsideCastling || type == QueensideCastling
               || (move.piece == piece && move.startingSquare == startingSquare && move.targetSquare == targetSquare
                   && ((type != Capture && type != PromotionWithCapture && type != EnPassant)
                       || move.capturedPiece == capturedPiece)
                   && ((type != Promotion && type != PromotionWithCapture)
                       || move.promotedPiece == promotedPiece)));
}

bool Move::parseCastlingNotation(const std::string &notation, MoveTypes &type) {
    if (notation == "0-0" || notation == "O-O") {
        type = KingsideCastling;
        return true;
    }
    if (notation == "0-0-0" || notation == "O-O-O") {
        type = QueensideCastling;
        return true;
    }
    return false;
}

[[nodiscard]] std::string Move::toLongAlgebraic(bool check, bool mate) const {
    std::string move;
    switch (type) {
        case KingsideCastling:
        case QueensideCastling:
            move = getCastlingNotation(type);
            break;
        default:
            bool isCapture = type == Capture || type == PromotionWithCapture || type == EnPassant;
            bool isPromotion = type == Promotion || type == PromotionWithCapture;
            bool isEnPassantCapture = type == EnPassant;
            move = Piece::toAlgebraic(piece) + startingSquare.toAlgebraic() +
                   (isCapture ? "x" + Piece::toAlgebraic(capturedPiece) : "-") + targetSquare.toAlgebraic() +
                   (isPromotion ? "=" + Piece::toAlgebraic(promotedPiece) : "") +
                   (isEnPassantCapture ? "e.p." : "");
            break;
    }
    return addSuffix(move, check, mate);
}

Move::Move(Pieces piece, Sides side, MoveTypes type, const Square &startingSquare, const Square &targetSquare,
           Pieces capturedPiece, Pieces promotedPiece) : piece(piece), side(side), type(type),
                                                         startingSquare(startingSquare), targetSquare(targetSquare),
                                                         capturedPiece(capturedPiece), promotedPiece(promotedPiece) {}

Move::Move() {}
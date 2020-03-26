#include <vector>

#include "analyzer.h"
#include "enums.h"
#include "exceptions.h"
#include "helper.h"
#include "matchers.h"
#include "piece.h"
#include "pieceCounts.h"
#include "position.h"
#include "requiredMoveMaps.h"
#include "validator.h"

void Validator::validateUserKings(const std::vector<Piece> &pieces) {
    bool sawKing = false;
    for (auto &piece : pieces) {
        if (piece.kind == King) {
            if (sawKing) {
                throw ManyKings();
            } else {
                sawKing = true;
            }
        }
    }
    if (!sawKing) {
        throw NoKings();
    }
}

void Validator::validateUserCounts(const PieceCounts &counts) {
    if (1 + counts.queen + counts.rook + counts.whiteSquareBishop +
        counts.blackSquareBishop + counts.knight + counts.pawn > 16) {
        throw TooManyPieces();
    }
    if (counts.pawn > 8) {
        throw TooManyPawns();
    }
    if (!validateCounts(counts)) {
        throw InvalidPieceCount();
    }
}

void Validator::validateUserPawns(const std::vector<Piece> &pieces) {
    for (auto &piece : pieces) {
        if (piece.kind == Pawn) {
            if (piece.square.rank == 0 || piece.square.rank == 7) {
                throw OutlierPawn();
            }
        }
    }
}

void Validator::validateUserEnPassant(const Position &position) {
    if (position.getEnPassant() == True) {
        int file = position.getEnPassantFile();
        Sides side = Helper::opposite(position.getTurn());
        int rank = side == White ? 3 : 4;
        int shift = side == White ? -1 : 1;
        Square square = Square(file, rank);
        if (!position.isPieceInSquare(square, side, Pawn)
            || !position.isSquareEmpty(Square(file, rank + shift))
            || !position.isSquareEmpty(Square(file, rank + 2 * shift))) {
            throw InvalidEnPassant(file);
        }
    }
}

void Validator::validateUserHalfMoves(const Position &position) {
    if (position.getHalfMoveLog()) {
        int halfMoves = position.getHalfMoveCounter();
        if (position.getEnPassant() == True && halfMoves > 0) {
            throw HalfMovesEnPassantViolation();
        }
    }
}

void Validator::validateUserFullMoves(const Position &position) {
    if (position.getFullMoveLog()) {
        int fullMoves = position.getFullMoveCounter();
        if (fullMoves < 1) {
            throw NegativeFullMoves();
        }
        if (position.getHalfMoveLog() && position.getHalfMoveCounter() >= position.getPlyCounter()) {
            throw HalfMovesExceedFullMoves();
        }
        if (fullMoves == 1 && position.getTurn() == White && !Analyzer::canBeStarting(position)) {
            throw InvalidStartingPosition();
        }
    }
}

bool Validator::validateCounts(const PieceCounts &counts) {
    int extraQueens = std::max(0, counts.queen - 1);
    int extraRooks = std::max(0, counts.rook - 2);
    int extraWhiteSquareBishops = std::max(0, counts.whiteSquareBishop - 1);
    int extraBlackSquareBishops = std::max(0, counts.blackSquareBishop - 1);
    int extraKnights = std::max(0, counts.knight - 2);
    int extraPieces = extraQueens + extraRooks + extraWhiteSquareBishops + extraBlackSquareBishops + extraKnights;
    return extraPieces + counts.pawn <= 8;
}

bool Validator::validateChecks(const Position &position) {
    return !Analyzer::isInCheck(position, Helper::opposite(position.getTurn()));
}

bool Validator::validateRequiredMoveNumber(const Position &position, Sides side) {
    if (!position.getFullMoveLog()) {
        return true;
    }
    int completedMoves = position.getCompletedMoves(side);
    int capturedOpposite = 16 - static_cast<int>(position.getPieces(Helper::opposite(side)).size());
    if (capturedOpposite > 0 && completedMoves <= capturedOpposite) { // We're using <= instead of < because the first
        return false;                                                 // move cannot be a capture
    }
    auto pawn = ZeroMatcher(Pawn, pawnMoveMap);
    auto king = ZeroMatcher(King, kingMoveMap);
    auto queen = SingleMatcher(Queen, queenMoveMap, queenPromotedMoveMap);
    auto leftBishop = SingleMatcher(Bishop, leftBishopMoveMap, leftBishopPromotedMoveMap);
    auto rightBishop = SingleMatcher(Bishop, rightBishopMoveMap, rightBishopPromotedMoveMap);
    auto knight = DoubleMatcher(Knight, knightMoveMap, knightPromotedMoveMap);
    auto rook = DoubleMatcher(Rook, rookMoveMap, rookPromotedMoveMap);
    for (auto &piece : position.getPieces(side)) {
        pawn.add(piece);
        king.add(piece);
        queen.add(piece);
        leftBishop.add(piece);
        rightBishop.add(piece);
        knight.add(piece);
        rook.add(piece);
    }
    int movesRequired = pawn.count() + king.count() +
                        queen.count() + leftBishop.count() + rightBishop.count() +
                        knight.count() + rook.count();
    return movesRequired <= completedMoves;
}

bool Validator::validateInitial(const Position &position) {
    return position.getTurn() != White || !position.getFullMoveLog() || position.getFullMoveCounter() > 1
           || Analyzer::canBeStarting(position);
}

bool Validator::validate(const Position &position) {
    return validateChecks(position)
           && validateCounts(position.getPieceCounts(White)) && validateCounts(position.getPieceCounts(Black))
           && validateRequiredMoveNumber(position, White) && validateRequiredMoveNumber(position, Black)
           && validateInitial(position);
}

std::pair<bool, std::string> Validator::validateAndStrictenUserPosition(Position &position) {
    for (auto &side : {White, Black}) {
        std::string sideString = Helper::sideToString(side, true);
        try {
            validateUserKings(position.getPieces(side));
        } catch (const NoKings &e) {
            return {false, sideString + " have no kings"};
        } catch (const ManyKings &e) {
            return {false, sideString + " have more than one king"};
        }
        try {
            validateUserCounts(position.getPieceCounts(side));
        } catch (const TooManyPieces &e) {
            return {false, sideString + " have too many pieces"};
        } catch (const TooManyPawns &e) {
            return {false, sideString + " have too many pawns"};
        } catch (const InvalidPieceCount &e) {
            return {false, sideString + "'s piece count is not possible in a legal game"};
        }
        try {
            validateUserPawns(position.getPieces(side));
        } catch (const OutlierPawn &e) {
            return {false, sideString + " pawn on a boundary rank"};
        }
    }

    if (position.getKing(White).isAdjacent(position.getKing(Black))) {
        return {false, "Kings are attacking each other"}; // Not strictly necessary, but might be clearer to user
    }

    if (!validateChecks(position)) {
        return {false, "Side moved last is in check"};
    }

    for (auto &side : {White, Black}) {
        for (auto &castlingSide : {Kingside, Queenside}) {
            if (!Analyzer::isInCastlingPosition(position, side, castlingSide)) {
                if (position.getCastling(side, castlingSide) == True) {
                    return {false, Helper::sideToString(side, true) + "'s " +
                                   Helper::castlingSideToString(castlingSide) +
                                   " castling is incorrectly defined as possible"};
                } else {
                    position.setCastling(side, castlingSide, False);
                }
            }
        }
    }

    try {
        validateUserEnPassant(position);
    } catch (const InvalidEnPassant &e) {
        return {false, e.what()};
    }

    try {
        validateUserHalfMoves(position);
    } catch (const HalfMovesEnPassantViolation &e) {
        return {false, e.what()};
    }

    try {
        validateUserFullMoves(position);
    } catch (const NegativeFullMoves &e) {
        return {false, e.what()};
    } catch (const HalfMovesExceedFullMoves &e) {
        return {false, e.what()};
    } catch (const InvalidStartingPosition &e) {
        return {false, e.what()};
    }

    return {true, ""};
}
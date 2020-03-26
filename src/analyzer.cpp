#include <string>
#include <vector>

#include "advancer.h"
#include "analyzer.h"
#include "enums.h"
#include "exceptions.h"
#include "FENParser.h"
#include "helper.h"
#include "move.h"
#include "piece.h"
#include "position.h"
#include "square.h"

constexpr char starting[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
constexpr char startingExtra[] = "w KQkq - 0 1";

bool Analyzer::isRangeEmpty(const Position &position, Square square, int fileStep, int rankStep, int max) {
    for (int i = 1; i < max; ++i) {
        square.file += fileStep;
        square.rank += rankStep;
        if (position.getSquareInfo(square).occupied) {
            return false;
        }
    }
    return true;
}

bool Analyzer::isAttackingAsRook(const Position &position, const Piece &piece, int fileDelta, int rankDelta) {
    bool fileAttack = fileDelta == 0 && isRangeEmpty(position, piece.square, 0, Helper::sgn(rankDelta), abs(rankDelta));
    bool rankAttack = rankDelta == 0 && isRangeEmpty(position, piece.square, Helper::sgn(fileDelta), 0, abs(fileDelta));
    return fileAttack || rankAttack;
}

bool Analyzer::isAttackingAsBishop(const Position &position, const Piece &piece, int fileDelta, int rankDelta) {
    return abs(fileDelta) == abs(rankDelta)
           && isRangeEmpty(position, piece.square, Helper::sgn(fileDelta), Helper::sgn(rankDelta), abs(fileDelta));
}

bool Analyzer::isAttacking(const Position &position, const Piece &piece, const Square &square) {
    int fileDelta = square.file - piece.square.file;
    int rankDelta = square.rank - piece.square.rank;
    switch (piece.kind) {
        case King:
            return abs(fileDelta) <= 1 && abs(rankDelta) <= 1;
        case Queen:
            return isAttackingAsRook(position, piece, fileDelta, rankDelta)
                   || isAttackingAsBishop(position, piece, fileDelta, rankDelta);
        case Rook:
            return isAttackingAsRook(position, piece, fileDelta, rankDelta);
        case Bishop:
            return isAttackingAsBishop(position, piece, fileDelta, rankDelta);
        case Knight:
            return (abs(fileDelta) == 1 && abs(rankDelta) == 2) || (abs(fileDelta) == 2 && abs(rankDelta) == 1);
        case Pawn:
            return abs(fileDelta) == 1 && rankDelta == (piece.side == White ? 1 : -1);
    }
}

void Analyzer::getLegalMoves(const Position &position, std::vector<Move> &moves, bool returnJustFirst) {
    std::vector<Move> allMoves;
    Advancer::enumerateMoves(position, allMoves);
    moves.clear();
    moves.reserve(returnJustFirst ? 1 : allMoves.size());
    for (auto &move : allMoves) {
        Position next = position;
        Advancer::advance(next, move);
        if (!isInCheck(next, position.getTurn())) {
            moves.emplace_back(move);
            if (returnJustFirst) {
                return;
            }
        }
    }
}

bool Analyzer::isUnderAttack(const Position &position, Sides side, const Square &square) {
    Sides opposite = Helper::opposite(side);
    for (auto &piece : position.getPieces(opposite)) {
        if (isAttacking(position, piece, square)) {
            return true;
        }
    }
    return false;
}

bool Analyzer::isInCheck(const Position &position, Sides side) {
    return isUnderAttack(position, side, position.getKing(side).square);
}

bool Analyzer::isInCheck(const Position &position) {
    return isInCheck(position, position.getTurn());
}

bool Analyzer::isCheckmated(const Position &position) {
    if (!isInCheck(position)) { // to eliminate stalemate
        return false;
    }
    std::vector<Move> moves;
    getLegalMoves(position, moves, true);
    return moves.empty();
}

bool Analyzer::isInCastlingPosition(const Position &position, Sides side, CastlingSides castlingSide,
                                    bool needBreakingMove, const Move *move) {
    int rank = side == White ? 0 : 7;
    int file = castlingSide == Kingside ? 7 : 0;
    return position.isPieceInSquare(Square(4, rank), side, King)
           && position.isPieceInSquare(Square(file, rank), side, Rook)
           && (!needBreakingMove
               || (move->side == side && move->startingSquare.rank == rank
                   && (move->startingSquare.file == 4 || move->startingSquare.file == file))
               || (move->side != side && move->targetSquare.file == file && move->targetSquare.rank == rank));
}

Position Analyzer::getStartingPosition() {
    std::string FEN = starting;
    FEN += " ";
    FEN += startingExtra;
    return FENParser::parse(FEN);
}

bool Analyzer::canBeStarting(const Position &position) {
    return position.getTurn() == White
           && (!position.getFullMoveLog() || position.getFullMoveCounter() == 1)
           && (!position.getHalfMoveLog() || position.getHalfMoveCounter() == 0)
           && position.getPieces(White).size() == 16 && position.getPieces(Black).size() == 16 // optimization
           && position.getCastling(White, Kingside) != False && position.getCastling(White, Queenside) != False
           && position.getCastling(Black, Kingside) != False && position.getCastling(Black, Queenside) != False
           && position.toFENPlacement() == starting;
}

Move Analyzer::interpretShortAlgebraic(const std::string &notation, const Position &position, bool &check, bool &mate) {
    bool castling = false, promotion = false, capture = false;
    int targetFile = -1, targetRank = -1, startingFile = -1, startingRank = -1;
    MoveTypes castlingType = KingsideCastling;
    Pieces piece = King, promotedPiece = King;

    check = false;
    mate = false;

    std::string left = notation;

    if (!left.empty()) {
        if (left.back() == '+') {
            left.pop_back();
            check = true;
        } else if (left.back() == '#') {
            left.pop_back();
            mate = true;
        }
    }

    castling = Move::parseCastlingNotation(left, castlingType);
    if (!castling) {
        if (left.size() >= 2 && left[left.size() - 2] == '=') {
            promotion = true;
            try {
                promotedPiece = Piece::fromAlgebraic(left.back());
            } catch (UnknownPiece &e) {
                throw AlgebraicInterpretationError(e.what());
            }
            left.pop_back();
            left.pop_back();
        }

        if (left.size() < 2) {
            throw AlgebraicInterpretationError("Unexpectedly short notation: " + notation);
        }

        try {
            targetRank = Square::rankFromAlgebraic(left.back());
        } catch (UnknownRank &e) {
            throw AlgebraicInterpretationError(e.what());
        }
        left.pop_back();

        try {
            targetFile = Square::fileFromAlgebraic(left.back());
        } catch (UnknownFile &e) {
            throw AlgebraicInterpretationError(e.what());
        }
        left.pop_back();

        if (!left.empty() && left.back() == 'x') {
            capture = true;
            left.pop_back();
        }

        try {
            switch (left.size()) {
                case 0:
                    piece = Pawn;
                    break;
                case 1:
                    if (Square::isValidFile(left[0])) {
                        piece = Pawn;
                        startingFile = Square::fileFromAlgebraic(left[0]);
                    } else {
                        piece = Piece::fromAlgebraic(left[0]);
                    }
                    break;
                case 2:
                    piece = Piece::fromAlgebraic(left[0]);
                    if (Square::isValidFile(left[1])) {
                        startingFile = Square::fileFromAlgebraic(left[1]);
                    } else if (Square::isValidRank(left[1])) {
                        startingRank = Square::rankFromAlgebraic(left[1]);
                    } else {
                        throw AlgebraicInterpretationError("Unrecognized disambiguation: " +
                                                           Helper::charToString(left[1]));
                    }
                    break;
                case 3:
                    piece = Piece::fromAlgebraic(left[0]);
                    try {
                        startingFile = Square::fileFromAlgebraic(left[1]);
                    } catch (UnknownFile &e) {
                        throw AlgebraicInterpretationError(e.what());
                    }
                    try {
                        startingRank = Square::rankFromAlgebraic(left[2]);
                    } catch (UnknownRank &e) {
                        throw AlgebraicInterpretationError(e.what());
                    }
                    break;
                default:
                    throw AlgebraicInterpretationError("Can't parse prefix: " + left);
            }
        } catch (UnknownPiece &e) {
            throw AlgebraicInterpretationError(e.what());
        }
    }

    std::vector<Move> legalMoves;
    getLegalMoves(position, legalMoves);
    int index = -1;
    for (int i = 0; i < legalMoves.size(); ++i) {
        bool match = true;
        Move move = legalMoves[i];

        if (castling) {
            match = move.type == castlingType;
        } else if (move.type == KingsideCastling || move.type == QueensideCastling || move.piece != piece
                   || move.targetSquare.file != targetFile || move.targetSquare.rank != targetRank
                   || (startingFile != -1 && move.startingSquare.file != startingFile)
                   || (startingRank != -1 && move.startingSquare.rank != startingRank)) {
            match = false;
        } else if (piece == Pawn) {
            if (promotion) {
                match = ((capture && move.type == PromotionWithCapture) || (!capture && move.type == Promotion))
                        && move.promotedPiece == promotedPiece;
            } else if (move.type == Promotion || move.type == PromotionWithCapture) {
                match = false;
            } else {
                match = (capture && (move.type == Capture || move.type == EnPassant))
                        || (!capture && move.type == SimpleMove);
            }
        } else if (capture) {
            match = (capture && move.type == Capture) || (!capture && move.type == SimpleMove);
        }

        if (match) {
            if (index != -1) {
                throw AlgebraicInterpretationError("Ambiguous move: " + notation + "; could be either " +
                                                   move.toLongAlgebraic(check, mate) + " or " +
                                                   legalMoves[index].toLongAlgebraic(check, mate));
            }
            index = i;
        }
    }

    if (index == -1) {
        throw AlgebraicInterpretationError("Cannot match notation against the available moves: " + notation);
    }

    return legalMoves[index];
}
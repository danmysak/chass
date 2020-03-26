#include <vector>

#include "analyzer.h"
#include "enums.h"
#include "helper.h"
#include "move.h"
#include "piece.h"
#include "position.h"
#include "retractor.h"
#include "square.h"

constexpr int movesBufferSize = 2500;

void Retractor::updatePieces(Position &position, const Move &move) {
    Sides side = move.side;
    Sides opposite = Helper::opposite(side);
    switch (move.type) {
        case SimpleMove:
            position.movePiece(move.targetSquare, move.startingSquare);
            break;
        case Promotion:
            position.removePiece(move.targetSquare);
            position.addPiece(move.startingSquare, Pawn, side);
            break;
        case Capture:
            position.movePiece(move.targetSquare, move.startingSquare);
            position.addPiece(move.targetSquare, move.capturedPiece, opposite);
            break;
        case PromotionWithCapture:
            position.removePiece(move.targetSquare);
            position.addPiece(move.startingSquare, Pawn, side);
            position.addPiece(move.targetSquare, move.capturedPiece, opposite);
            break;
        case EnPassant: {
            position.movePiece(move.targetSquare, move.startingSquare);
            Square square = Square(move.targetSquare.file, side == White ? 4 : 3);
            position.addPiece(square, Pawn, opposite);
            break;
        }
        case KingsideCastling:
        case QueensideCastling: {
            bool kingside = move.type == KingsideCastling;
            int firstRank = side == White ? 0 : 7;
            position.movePiece(move.targetSquare, move.startingSquare);
            position.movePiece(Square(kingside ? 5 : 3, firstRank), Square(kingside ? 7 : 0, firstRank));
            break;
        }
    }
}

void Retractor::updateCastling(Position &position, const Move &move) {
    Sides side = move.side;
    Sides opposite = Helper::opposite(side);
    Ternary kingCastling = position.getCastling(side, Kingside);
    Ternary queenCastling = position.getCastling(side, Queenside);
    switch (move.type) {
        case SimpleMove:
        case Capture:
            if (kingCastling != True) {
                Ternary state = Analyzer::isInCastlingPosition(position, side, Kingside, kingCastling == False, &move)
                        ? Unknown : False;
                position.setCastling(side, Kingside, state);
            }
            if (queenCastling != True) {
                Ternary state = Analyzer::isInCastlingPosition(position, side, Queenside, queenCastling == False, &move)
                        ? Unknown : False;
                position.setCastling(side, Queenside, state);
            }
            break;
        case KingsideCastling: {
            position.setCastling(side, Kingside, True);
            Ternary state = Analyzer::isInCastlingPosition(position, side, Queenside) ? Unknown : False;
            position.setCastling(side, Queenside, state);
            break;
        }
        case QueensideCastling: {
            Ternary state = Analyzer::isInCastlingPosition(position, side, Kingside) ? Unknown : False;
            position.setCastling(side, Kingside, state);
            position.setCastling(side, Queenside, True);
            break;
        }
        default:
            break;
    }
    switch (move.type) {
        case Capture:
        case PromotionWithCapture:
            if (Analyzer::isInCastlingPosition(position, opposite, Kingside, true, &move)) {
                position.setCastling(opposite, Kingside, Unknown);
            }
            if (Analyzer::isInCastlingPosition(position, opposite, Queenside, true, &move)) {
                position.setCastling(opposite, Queenside, Unknown);
            }
            break;
        default:
            break;
    }
}

void Retractor::updateEnPassant(Position &position, const Move &move) {
    if (move.type == EnPassant) {
        position.setEnPassant(True, move.targetSquare.file);
    } else {
        position.setEnPassant(Unknown);
    }
}

void Retractor::updateMoves(Position &position, const Move &move) {
    position.setTurn(move.side);

    if (position.getHalfMoveLog()) {
        if (position.getHalfMoveCounter() == 0) {
            position.setHalfMoves(false);
        } else {
            position.decrementHalfMoves();
        }
    }

    if (position.getFullMoveLog() && move.side == Black) {
        position.decrementFullMoves();
    }
}

Move Retractor::constructMove(const Piece &piece, MoveTypes type, const Square &startingSquare, Pieces capturedPiece) {
    return Move(piece.kind, piece.side, type, startingSquare, piece.square, capturedPiece);
}

void Retractor::appendCaptureMove(const Move &move, Pieces capturedPiece, std::vector<Move> &moves) {
    Move captureMove = move;
    captureMove.capturedPiece = capturedPiece;
    moves.emplace_back(captureMove);
}

void Retractor::enumerateCaptureMoves(const Move &move, std::vector<Move> &moves) {
    appendCaptureMove(move, Queen, moves);
    appendCaptureMove(move, Rook, moves);
    appendCaptureMove(move, Bishop, moves);
    appendCaptureMove(move, Knight, moves);
    if (move.targetSquare.rank != 0 && move.targetSquare.rank != 7) {
        appendCaptureMove(move, Pawn, moves);
    }
}

void Retractor::enumeratePotentialCaptureMoves(const Piece &piece, const Square &square, Ternary pawnOrCapture,
                                               std::vector<Move> &moves) {
    if (pawnOrCapture != True) {
        moves.emplace_back(constructMove(piece, SimpleMove, square));
    }
    if (pawnOrCapture != False) {
        enumerateCaptureMoves(constructMove(piece, Capture, square), moves);
    }
}

void Retractor::enumerateKingMoves(const Position &position, const Piece &piece, Ternary pawnOrCapture,
                                   std::vector<Move> &moves) {
    for (int fileDelta = -1; fileDelta <= 1; ++fileDelta) {
        for (int rankDelta = -1; rankDelta <= 1; ++rankDelta) {
            if (fileDelta == 0 && rankDelta == 0) {
                continue;
            }
            Square square = piece.square.shift(fileDelta, rankDelta);
            if (position.isOnBoard(square) && position.isSquareEmpty(square)) {
                enumeratePotentialCaptureMoves(piece, square, pawnOrCapture, moves);
            }
        }
    }
    int firstRank = piece.side == White ? 0 : 7;
    if (pawnOrCapture != True && piece.square.rank == firstRank) {
        if (piece.square.file == 6) {
            Square initialSquare = piece.square.shift(-2, 0);
            Square rookSquare = piece.square.shift(-1, 0);
            if (position.isPieceInSquare(rookSquare, piece.side, Rook)
                && position.isSquareEmpty(initialSquare)
                && position.isSquareEmpty(piece.square.shift(1, 0))
                && !Analyzer::isUnderAttack(position, piece.side, rookSquare)
                && !Analyzer::isUnderAttack(position, piece.side, initialSquare)) {
                moves.emplace_back(constructMove(piece, KingsideCastling, initialSquare));
            }
        } else if (piece.square.file == 2) {
            Square initialSquare = piece.square.shift(2, 0);
            Square rookSquare = piece.square.shift(1, 0);
            if (position.isPieceInSquare(rookSquare, piece.side, Rook)
                && position.isSquareEmpty(initialSquare)
                && position.isSquareEmpty(piece.square.shift(-1, 0))
                && position.isSquareEmpty(piece.square.shift(-2, 0))
                && !Analyzer::isUnderAttack(position, piece.side, rookSquare)
                && !Analyzer::isUnderAttack(position, piece.side, initialSquare)) {
                moves.emplace_back(constructMove(piece, QueensideCastling, initialSquare));
            }
        }
    }
}

void Retractor::enumerateLinearMoves(const Position &position, const Piece &piece, int fileDirection, int rankDirection,
                                     Ternary pawnOrCapture, std::vector<Move> &moves) {
    int fileDelta = fileDirection;
    int rankDelta = rankDirection;
    while (true) {
        Square square = piece.square.shift(fileDelta, rankDelta);
        if (position.isOnBoard(square) && position.isSquareEmpty(square)) {
            enumeratePotentialCaptureMoves(piece, square, pawnOrCapture, moves);
        } else {
            break;
        }
        fileDelta += fileDirection;
        rankDelta += rankDirection;
    }
}

void Retractor::enumerateRookLikeMoves(const Position &position, const Piece &piece, Ternary pawnOrCapture,
                                       std::vector<Move> &moves) {
    enumerateLinearMoves(position, piece, -1, 0, pawnOrCapture, moves);
    enumerateLinearMoves(position, piece, 1, 0, pawnOrCapture, moves);
    enumerateLinearMoves(position, piece, 0, -1, pawnOrCapture, moves);
    enumerateLinearMoves(position, piece, 0, 1, pawnOrCapture, moves);
}

void Retractor::enumerateBishopLikeMoves(const Position &position, const Piece &piece, Ternary pawnOrCapture,
                                         std::vector<Move> &moves) {
    enumerateLinearMoves(position, piece, -1, -1, pawnOrCapture, moves);
    enumerateLinearMoves(position, piece, 1, -1, pawnOrCapture, moves);
    enumerateLinearMoves(position, piece, -1, 1, pawnOrCapture, moves);
    enumerateLinearMoves(position, piece, 1, 1, pawnOrCapture, moves);
}

void Retractor::enumerateKnightMoves(const Position &position, const Piece &piece, Ternary pawnOrCapture,
                                     std::vector<Move> &moves) {
    for (int fileDelta = -2; fileDelta <= 2; ++fileDelta) {
        if (fileDelta == 0) {
            continue;
        }
        int rankDelta = abs(fileDelta) - 3;
        while (true) {
            Square square = piece.square.shift(fileDelta, rankDelta);
            if (position.isOnBoard(square) && position.isSquareEmpty(square)) {
                enumeratePotentialCaptureMoves(piece, square, pawnOrCapture, moves);
            }
            if (rankDelta > 0) {
                break;
            }
            rankDelta = -rankDelta;
        }
    }
}

void Retractor::enumeratePawnMoves(const Position &position, const Piece &piece, Ternary enPassant,
                                   std::vector<Move> &moves) {
    int initialRank = piece.side == White ? 1 : 6;
    if (piece.square.rank == initialRank) {
        return;
    }
    int enPassantRank = piece.side == White ? 3 : 4;
    int shift = piece.side == White ? -1 : 1;
    if (enPassant == True) {
        if (piece.square.file == position.getEnPassantFile() && piece.square.rank == enPassantRank) {
            moves.emplace_back(constructMove(piece, SimpleMove, piece.square.shift(0, 2 * shift)));
        }
        return;
    }
    if (enPassant != False) {
        if (piece.square.rank == enPassantRank
            && position.isSquareEmpty(piece.square.shift(0, shift))
            && position.isSquareEmpty(piece.square.shift(0, 2 * shift))) {
            moves.emplace_back(constructMove(piece, SimpleMove, piece.square.shift(0, 2 * shift)));
        }
    }
    {
        Square square = piece.square.shift(0, shift);
        if (position.isSquareEmpty(square)) {
            moves.emplace_back(constructMove(piece, SimpleMove, square));
        }
    }
    for (int fileDelta = -1; fileDelta <= 1; fileDelta += 2) {
        Square square = piece.square.shift(fileDelta, shift);
        if (!position.isOnBoard(square)) {
            continue;
        }
        if (position.isSquareEmpty(square)) {
            enumerateCaptureMoves(constructMove(piece, Capture, square), moves);
            if (square.rank == enPassantRank - shift
                && position.isSquareEmpty(piece.square.shift(0, shift))
                && position.isSquareEmpty(piece.square.shift(0, -shift))) {
                moves.emplace_back(constructMove(piece, EnPassant, square, Pawn));
            }
        }
    }
}

void Retractor::enumeratePromotionMoves(const Position &position, const Piece &piece, std::vector<Move> &moves) {
    int lastRank = piece.side == White ? 7 : 0;
    if (piece.square.rank == lastRank && piece.kind != King) {
        int shift = piece.side == White ? -1 : 1;
        {
            Square square = piece.square.shift(0, shift);
            if (position.isSquareEmpty(square)) {
                moves.emplace_back(Move(Pawn, piece.side, Promotion, square, piece.square, King, piece.kind));
            }
        }
        for (int fileDelta = -1; fileDelta <= 1; fileDelta += 2) {
            Square square = piece.square.shift(fileDelta, shift);
            if (position.isOnBoard(square) && position.isSquareEmpty(square)) {
                enumerateCaptureMoves(Move(Pawn, piece.side, PromotionWithCapture, square, piece.square,
                                           King, piece.kind), moves);
            }
        }
    }
}

void Retractor::enumerateMoves(const Position &position, std::vector<Move> &moves) {
    moves.clear();
    if (position.getFullMoveLog() && position.getFullMoveCounter() == 1 && position.getTurn() == White) {
        return;
    }
    moves.reserve(movesBufferSize);
    const std::vector<Piece> &pieces = position.getPieces(Helper::opposite(position.getTurn()));
    Ternary pawnOrCapture = position.getHalfMoveLog() ? (position.getHalfMoveCounter() == 0 ? True : False)
                                                      : Unknown;
    Ternary enPassant = position.getEnPassant();
    for (auto &piece : pieces) {
        switch (piece.kind) {
            case King:
                if (enPassant != True) {
                    enumerateKingMoves(position, piece, pawnOrCapture, moves);
                }
                break;
            case Queen:
                if (enPassant != True) {
                    enumerateRookLikeMoves(position, piece, pawnOrCapture, moves);
                    enumerateBishopLikeMoves(position, piece, pawnOrCapture, moves);
                }
                break;
            case Rook:
                if (enPassant != True) {
                    enumerateRookLikeMoves(position, piece, pawnOrCapture, moves);
                }
                break;
            case Bishop:
                if (enPassant != True) {
                    enumerateBishopLikeMoves(position, piece, pawnOrCapture, moves);
                }
                break;
            case Knight:
                if (enPassant != True) {
                    enumerateKnightMoves(position, piece, pawnOrCapture, moves);
                }
                break;
            case Pawn:
                if (pawnOrCapture != False) {
                    enumeratePawnMoves(position, piece, enPassant, moves);
                }
                break;
        }
        if (pawnOrCapture != False && enPassant != True) {
            enumeratePromotionMoves(position, piece, moves);
        }
    }
}

void Retractor::retract(Position &position, const Move &move) {
    updatePieces(position, move);
    updateCastling(position, move);
    updateEnPassant(position, move);
    updateMoves(position, move);
}
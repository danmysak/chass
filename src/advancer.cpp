#include <vector>

#include "advancer.h"
#include "analyzer.h"
#include "enums.h"
#include "helper.h"
#include "move.h"
#include "piece.h"
#include "position.h"

constexpr int movesBufferSize = 400;

void Advancer::updatePieces(Position &position, const Move &move) {
    Sides side = move.side;
    switch (move.type) {
        case SimpleMove:
            position.movePiece(move.startingSquare, move.targetSquare);
            break;
        case Promotion:
            position.removePiece(move.startingSquare);
            position.addPiece(move.targetSquare, move.promotedPiece, side);
            break;
        case Capture:
            position.removePiece(move.targetSquare);
            position.movePiece(move.startingSquare, move.targetSquare);
            break;
        case PromotionWithCapture:
            position.removePiece(move.targetSquare);
            position.removePiece(move.startingSquare);
            position.addPiece(move.targetSquare, move.promotedPiece, side);
            break;
        case EnPassant: {
            Square square = Square(move.targetSquare.file, side == White ? 4 : 3);
            position.removePiece(square);
            position.movePiece(move.startingSquare, move.targetSquare);
            break;
        }
        case KingsideCastling:
        case QueensideCastling: {
            bool kingside = move.type == KingsideCastling;
            int firstRank = side == White ? 0 : 7;
            position.movePiece(move.startingSquare, move.targetSquare);
            position.movePiece(Square(kingside ? 7 : 0, firstRank), Square(kingside ? 5 : 3, firstRank));
            break;
        }
    }
}

void Advancer::updateCastling(Position &position, const Move &move) {
    for (auto &side : {White, Black}) {
        for (auto &castlingSide : {Kingside, Queenside}) {
            if (!Analyzer::isInCastlingPosition(position, side, castlingSide)) {
                position.setCastling(side, castlingSide, False);
            }
        }
    }
}

void Advancer::updateEnPassant(Position &position, const Move &move) {
    if (move.piece == Pawn && abs(move.startingSquare.rank - move.targetSquare.rank) == 2) {
        position.setEnPassant(True, move.startingSquare.file);
    } else {
        position.setEnPassant(False);
    }
}

void Advancer::updateMoves(Position &position, const Move &move) {
    position.setTurn(Helper::opposite(move.side));

    if (position.getHalfMoveLog()) {
        if (move.piece == Pawn || move.type == Capture) {
            position.setHalfMoves(true, 0);
        } else {
            position.incrementHalfMoves();
        }
    }

    if (position.getFullMoveLog() && move.side == Black) {
        position.incrementFullMoves();
    }
}

Move Advancer::constructMove(const Piece &piece, MoveTypes type, const Square &targetSquare, Pieces capturedPiece) {
    return Move(piece.kind, piece.side, type, piece.square, targetSquare, capturedPiece);
}

void Advancer::enumerateKingMoves(const Position &position, const Piece &piece, std::vector<Move> &moves) {
    for (int fileDelta = -1; fileDelta <= 1; ++fileDelta) {
        for (int rankDelta = -1; rankDelta <= 1; ++rankDelta) {
            if (fileDelta == 0 && rankDelta == 0) {
                continue;
            }
            Square square = piece.square.shift(fileDelta, rankDelta);
            if (!position.isOnBoard(square)) {
                continue;
            }
            SquareInfo info = position.getSquareInfo(square);
            if (!info.occupied || info.side != piece.side) {
                moves.emplace_back(constructMove(piece, info.occupied ? Capture : SimpleMove, square,
                                                 info.occupied ? position.getPiece(info).kind : King));
            }
        }
    }
    // Non-False castling indicator implies that the king and rook are in proper positions
    if (position.getCastling(piece.side, Kingside) != False) {
        if (position.isSquareEmpty(piece.square.shift(1, 0))
            && position.isSquareEmpty(piece.square.shift(2, 0))
            && !Analyzer::isUnderAttack(position, piece.side, piece.square)
            && !Analyzer::isUnderAttack(position, piece.side, piece.square.shift(1, 0))) {
            moves.emplace_back(constructMove(piece, KingsideCastling, piece.square.shift(2, 0)));
        }
    }
    // Checking whether the king's new position is in check is done on a general basis
    if (position.getCastling(piece.side, Queenside) != False) {
        if (position.isSquareEmpty(piece.square.shift(-1, 0))
            && position.isSquareEmpty(piece.square.shift(-2, 0))
            && position.isSquareEmpty(piece.square.shift(-3, 0))
            && !Analyzer::isUnderAttack(position, piece.side, piece.square)
            && !Analyzer::isUnderAttack(position, piece.side, piece.square.shift(-1, 0))) {
            moves.emplace_back(constructMove(piece, QueensideCastling, piece.square.shift(-2, 0)));
        }
    }
}

void Advancer::enumerateLinearMoves(const Position &position, const Piece &piece, int fileDirection, int rankDirection,
                                    std::vector<Move> &moves) {
    int fileDelta = fileDirection;
    int rankDelta = rankDirection;
    while (true) {
        Square square = piece.square.shift(fileDelta, rankDelta);
        if (!position.isOnBoard(square)) {
            break;
        }
        SquareInfo info = position.getSquareInfo(square);
        if (info.occupied) {
            if (info.side != piece.side) {
                moves.emplace_back(constructMove(piece, Capture, square, position.getPiece(info).kind));
            }
            break;
        } else {
            moves.emplace_back(constructMove(piece, SimpleMove, square));
        }
        fileDelta += fileDirection;
        rankDelta += rankDirection;
    }
}

void Advancer::enumerateRookLikeMoves(const Position &position, const Piece &piece, std::vector<Move> &moves) {
    enumerateLinearMoves(position, piece, -1, 0, moves);
    enumerateLinearMoves(position, piece, 1, 0, moves);
    enumerateLinearMoves(position, piece, 0, -1, moves);
    enumerateLinearMoves(position, piece, 0, 1, moves);
}

void Advancer::enumerateBishopLikeMoves(const Position &position, const Piece &piece, std::vector<Move> &moves) {
    enumerateLinearMoves(position, piece, -1, -1, moves);
    enumerateLinearMoves(position, piece, 1, -1, moves);
    enumerateLinearMoves(position, piece, -1, 1, moves);
    enumerateLinearMoves(position, piece, 1, 1, moves);
}

void Advancer::enumerateKnightMoves(const Position &position, const Piece &piece, std::vector<Move> &moves) {
    for (int fileDelta = -2; fileDelta <= 2; ++fileDelta) {
        if (fileDelta == 0) {
            continue;
        }
        int rankDelta = abs(fileDelta) - 3;
        while (true) {
            Square square = piece.square.shift(fileDelta, rankDelta);
            if (position.isOnBoard(square)) {
                SquareInfo info = position.getSquareInfo(square);
                if (!info.occupied || info.side != piece.side) {
                    moves.emplace_back(constructMove(piece, info.occupied ? Capture : SimpleMove, square,
                                                     info.occupied ? position.getPiece(info).kind : King));
                }
            }
            if (rankDelta > 0) {
                break;
            }
            rankDelta = -rankDelta;
        }
    }
}

void Advancer::appendPromotionMove(const Move &move, Pieces promotedPiece, std::vector<Move> &moves) {
    Move promotionMove = move;
    promotionMove.promotedPiece = promotedPiece;
    moves.emplace_back(promotionMove);
}

void Advancer::enumeratePromotionMoves(const Move &move, std::vector<Move> &moves) {
    appendPromotionMove(move, Queen, moves);
    appendPromotionMove(move, Rook, moves);
    appendPromotionMove(move, Bishop, moves);
    appendPromotionMove(move, Knight, moves);
}

void Advancer::enumeratePawnMoves(const Position &position, const Piece &piece, std::vector<Move> &moves) {
    Ternary enPassant = position.getEnPassant();
    int enPassantFile = position.getEnPassantFile();
    int initialRank = piece.side == White ? 1 : 6;
    int enPassantRank = piece.side == White ? 4 : 3;
    int lastRank = piece.side == White ? 7 : 0;
    int direction = piece.side == White ? 1 : -1;
    {
        Square square = piece.square.shift(0, direction);
        if (position.isSquareEmpty(square)) {
            if (square.rank == lastRank) {
                enumeratePromotionMoves(constructMove(piece, Promotion, square), moves);
            } else {
                moves.emplace_back(constructMove(piece, SimpleMove, square));
                if (piece.square.rank == initialRank) {
                    Square forwardSquare = square.shift(0, direction);
                    if (position.isSquareEmpty(forwardSquare)) {
                        moves.emplace_back(constructMove(piece, SimpleMove, forwardSquare));
                    }
                }
            }
        }
    }
    for (int fileDelta = -1; fileDelta <= 1; fileDelta += 2) {
        Square square = piece.square.shift(fileDelta, direction);
        if (!position.isOnBoard(square)) {
            continue;
        }
        SquareInfo info = position.getSquareInfo(square);
        if (info.occupied) {
            if (info.side != piece.side) {
                if (square.rank == lastRank) {
                    enumeratePromotionMoves(constructMove(piece, PromotionWithCapture, square,
                                                          position.getPiece(info).kind), moves);
                } else {
                    moves.emplace_back(constructMove(piece, Capture, square, position.getPiece(info).kind));
                }
            }
        } else if (piece.square.rank == enPassantRank) {
            if ((enPassant == True && enPassantFile == square.file)
                || (enPassant == Unknown
                    && position.isPieceInSquare(square.shift(0, -direction), Helper::opposite(piece.side), Pawn)
                    && position.isSquareEmpty(square.shift(0, direction)))) {
                moves.emplace_back(constructMove(piece, EnPassant, square, Pawn));
            }
        }
    }
}

void Advancer::enumerateMoves(const Position &position, std::vector<Move> &moves) {
    moves.clear();
    moves.reserve(movesBufferSize);
    const std::vector<Piece> &pieces = position.getPieces(position.getTurn());
    for (auto &piece : pieces) {
        switch (piece.kind) {
            case King:
                enumerateKingMoves(position, piece, moves);
                break;
            case Queen:
                enumerateRookLikeMoves(position, piece, moves);
                enumerateBishopLikeMoves(position, piece, moves);
                break;
            case Rook:
                enumerateRookLikeMoves(position, piece, moves);
                break;
            case Bishop:
                enumerateBishopLikeMoves(position, piece, moves);
                break;
            case Knight:
                enumerateKnightMoves(position, piece, moves);
                break;
            case Pawn:
                enumeratePawnMoves(position, piece, moves);
                break;
        }
    }
}

void Advancer::advance(Position &position, const Move &move) {
    updatePieces(position, move);
    updateCastling(position, move);
    updateEnPassant(position, move);
    updateMoves(position, move);
}
#include <string>

#include "enums.h"
#include "exceptions.h"
#include "FENParser.h"
#include "helper.h"
#include "position.h"

constexpr int maxCounter = 3e4; // Otherwise there might be problems with position packing

void FENParser::ensureCharsLeft(const std::string &FEN, int cursor, bool showLastSpaceWarning) {
    if (cursor >= FEN.length()) {
        throw FENParseError("Unexpected end of record after position " + std::to_string(cursor) +
                            (showLastSpaceWarning ? " (did you leave whitespace after the notation?)" : ""));
    }
}

bool FENParser::tryReadSpace(const std::string &FEN, int &cursor) {
    if (cursor >= FEN.length()) {
        return false;
    }
    if (FEN[cursor] != ' ') {
        throw FENParseError("Expected space at position " + std::to_string(cursor + 1));
    }
    ++cursor;
    return true;
}

std::pair<Pieces, Sides> FENParser::parsePiece(char c) {
    Sides side = isupper(c) ? White : Black;
    switch (tolower(c)) {
        case 'p':
            return {Pawn, side};
        case 'n':
            return {Knight, side};
        case 'b':
            return {Bishop, side};
        case 'r':
            return {Rook, side};
        case 'q':
            return {Queen, side};
        case 'k':
            return {King, side};
    }
    throw UnknownPiece(c);
}

void FENParser::readPlacement(const std::string &FEN, int &cursor, Position &position) {
    int rank = 7, file = 0;
    while (true) {
        if (cursor >= FEN.length() || FEN[cursor] == ' ') {
            throw FENParseError("Piece placement description ended unexpectedly after position " +
                                std::to_string(cursor));
        }
        char c = FEN[cursor];
        if (file == 8 && c != '/') {
            throw FENParseError("Too many squares in piece placement description for rank " + std::to_string(rank + 1));
        }
        if (c == '/') {
            if (file == 8) {
                file = 0;
                --rank;
            } else {
                throw FENParseError("Too few squares in piece placement description for rank " +
                                    std::to_string(rank + 1));
            }
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
            if (file > 8) {
                throw FENParseError("Too many squares in piece placement description for rank " +
                                    std::to_string(rank + 1));
            }
        } else {
            try {
                auto [kind, side] = parsePiece(c);
                position.addPiece(Square(file, rank), kind, side);
            } catch (const UnknownPiece &e) {
                throw FENParseError("Invalid character '" + Helper::charToString(c) + "' at position " +
                                    std::to_string(cursor + 1) + " in piece placement description");
            }
            ++file;
        }
        ++cursor;
        if (file == 8 && rank == 0) {
            break;
        }
    }
}

void FENParser::readSide(const std::string &FEN, int &cursor, Position &position) {
    if (cursor >= FEN.length()) {
        throw FENParseError("Turn must be specified for the position");
    }
    if (FEN[cursor] != ' ') {
        throw FENParseError("Expected space at position " + std::to_string(cursor + 1));
    }
    ++cursor;
    ensureCharsLeft(FEN, cursor);
    char c = FEN[cursor];
    switch (c) {
        case 'w':
            position.setTurn(White);
            break;
        case 'b':
            position.setTurn(Black);
            break;
        default:
            throw FENParseError("Invalid character '" + Helper::charToString(c) + "' at position " +
                                std::to_string(cursor + 1) + " (expected 'w' for white or 'b' for black)");
    }
    ++cursor;
}

void FENParser::readCastling(const std::string &FEN, int &cursor, Position &position) {
    if (!tryReadSpace(FEN, cursor)) {
        return;
    }
    ensureCharsLeft(FEN, cursor, true);
    if (FEN[cursor] == '?') {
        ++cursor;
    } else {
        position.setCastling(White, Kingside, False);
        position.setCastling(White, Queenside, False);
        position.setCastling(Black, Kingside, False);
        position.setCastling(Black, Queenside, False);
        if (FEN[cursor] == '-') {
            ++cursor;
        } else {
            if (FEN[cursor] == ' ') {
                throw FENParseError("Unexpected space at position " +
                                    std::to_string(cursor + 1) + " (expected 'K', 'Q', 'k', or 'q'; " +
                                    "you can also use '-' or '?' for the whole castling slot)");
            }
            while (cursor < FEN.length()) {
                char c = FEN[cursor];
                if (c == ' ') {
                    break;
                }
                switch (c) {
                    case 'K':
                        position.setCastling(White, Kingside, True);
                        break;
                    case 'Q':
                        position.setCastling(White, Queenside, True);
                        break;
                    case 'k':
                        position.setCastling(Black, Kingside, True);
                        break;
                    case 'q':
                        position.setCastling(Black, Queenside, True);
                        break;
                    default:
                        throw FENParseError("Invalid character '" + Helper::charToString(c) + "' at position " +
                                            std::to_string(cursor + 1) + " (expected 'K', 'Q', 'k', or 'q'; " +
                                            "you can also use '-' or '?' for the whole castling slot)");
                }
                ++cursor;
            }
        }
    }
}

void FENParser::readEnPassant(const std::string &FEN, int &cursor, Position &position) {
    if (!tryReadSpace(FEN, cursor)) {
        return;
    }
    ensureCharsLeft(FEN, cursor, true);
    char c = FEN[cursor];
    ++cursor;
    if (c == '-') {
        position.setEnPassant(False);
    } else if (c >= 'a' && c <= 'h') {
        position.setEnPassant(True, c - 'a');
        if (cursor >= FEN.length()) {
            throw FENParseError("Unexpected end of record after position " + std::to_string(cursor) +
                                " (expected rank of the en passant square)");
        }
        char expectedRank = position.getTurn() == White ? '6' : '3';
        char rank = FEN[cursor];
        if (rank != expectedRank) {
            throw FENParseError("Unexpected rank '" + Helper::charToString(rank) + "' at position " +
                                std::to_string(cursor + 1) +
                                " (expected '" + Helper::charToString(expectedRank) + "')");
        }
        ++cursor;
    } else if (c != '?') {
        throw FENParseError("Invalid character '" + Helper::charToString(c) + "' at position " +
                            std::to_string(cursor) + " (expected '-', '?', or file 'a' through 'h' " +
                            "for the en passant square)");
    }
}

void FENParser::readMoves(const std::string &FEN, int &cursor, Position &position, bool halfMoves) {
    if (!tryReadSpace(FEN, cursor)) {
        return;
    }
    ensureCharsLeft(FEN, cursor, true);
    if (FEN[cursor] == '?') {
        ++cursor;
        return;
    }
    if (FEN[cursor] == ' ') {
        throw FENParseError("Unexpected space at position " + std::to_string(cursor + 1) +
                            " (expected " + (halfMoves ? "half" : "full") + "-move counter instead)");
    }
    int moves = 0;
    while (cursor < FEN.length()) {
        char c = FEN[cursor];
        if (c == ' ') {
            break;
        }
        if (c >= '0' && c <= '9') {
            moves = moves * 10 + (c - '0');
            if (moves > maxCounter) {
                throw FENParseError((std::string)(halfMoves ? "Half" : "Full") + "-move counter is too large");
            }
            ++cursor;
        } else {
            throw FENParseError("Invalid character '" + Helper::charToString(c) + "' at position " +
                                std::to_string(cursor + 1) + " (expected a digit of the " +
                                (halfMoves ? "half" : "full") + "-move counter; you can also use '?' for the slot)");
        }
    }
    if (halfMoves) {
        position.setHalfMoves(true, moves);
    } else {
        position.setFullMoves(true, moves);
    }
}

Position FENParser::parse(std::string &FEN) {
    Position position = {};
    int cursor = 0;
    if (FEN.empty()) {
        throw FENParseError("No input was provided");
    }
    readPlacement(FEN, cursor, position);
    readSide(FEN, cursor, position);
    readCastling(FEN, cursor, position);
    readEnPassant(FEN, cursor, position);
    readMoves(FEN, cursor, position, true);
    readMoves(FEN, cursor, position, false);
    while (true) {
        if (cursor == FEN.length() || FEN[cursor] != ' ') {
            break;
        }
        cursor++;
    }
    if (cursor != FEN.length()) {
        throw FENParseError("Unexpected continuation of record after position " + std::to_string(cursor));
    }
    return position;
}
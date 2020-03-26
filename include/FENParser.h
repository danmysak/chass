#ifndef CHASS_FENREADER_H
#define CHASS_FENREADER_H

#include <string>

#include "enums.h"
#include "position.h"

class FENParser {
    static void ensureCharsLeft(const std::string &FEN, int pos, bool showLastSpaceWarning = false);
    static bool tryReadSpace(const std::string &FEN, int &pos);
    static std::pair<Pieces, Sides> parsePiece(char c);
    static void readPlacement(const std::string &FEN, int &pos, Position &position);
    static void readSide(const std::string &FEN, int &pos, Position &position);
    static void readCastling(const std::string &FEN, int &pos, Position &position);
    static void readEnPassant(const std::string &FEN, int &pos, Position &position);
    static void readMoves(const std::string &FEN, int &pos, Position &position, bool halfMoves);

public:
    static Position parse(std::string &FEN);
};

#endif // CHASS_FENREADER_H
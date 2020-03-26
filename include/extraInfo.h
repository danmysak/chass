#ifndef CHASS_EXTRAINFO_H
#define CHASS_EXTRAINFO_H

#include "enums.h"

struct ExtraInfo {
    Ternary whiteKingCastling, whiteQueenCastling;
    Ternary blackKingCastling, blackQueenCastling;
    Ternary enPassant;
    int enPassantFile;

    ExtraInfo() : whiteKingCastling(Unknown), whiteQueenCastling(Unknown),
                  blackKingCastling(Unknown), blackQueenCastling(Unknown),
                  enPassant(Unknown), enPassantFile(0) {}
};

#endif // CHASS_EXTRAINFO_H
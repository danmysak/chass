#ifndef CHASS_MATCHERS_H
#define CHASS_MATCHERS_H

#include "enums.h"
#include "piece.h"

class ZeroMatcher {
private:
    Pieces type;
    const int (*map)[8][8];
    int counter;
public:
    ZeroMatcher(Pieces type, const int (&map)[8][8]);
    void add(const Piece &piece);
    int count();
};

class SingleMatcher {
private:
    Pieces type;
    const int (*map)[8][8];
    const int (*promoted)[8][8];
    int sumPromoted, maxDifference;
public:
    SingleMatcher(Pieces type, const int (&map)[8][8], const int (&promoted)[8][8]);
    void add(const Piece &piece);
    int count();
};

class DoubleMatcher {
private:
    Pieces type;
    const int (*map)[8][8];
    const int (*promoted)[8][8];
    int sumPromoted, totalPieces;
    int firstLeftMax, secondLeftMax, firstRightMax, secondRightMax;
    int leftIndex, rightIndex;
public:
    DoubleMatcher(Pieces type, const int (&map)[8][8], const int (&promoted)[8][8]);
    void add(const Piece &piece);
    int count();
};

#endif // CHASS_MATCHERS_H
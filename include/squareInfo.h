#ifndef CHASS_SQUAREINFO_H
#define CHASS_SQUAREINFO_H

#include "enums.h"

struct SquareInfo {
    bool occupied;
    Sides side;
    int index;

    explicit SquareInfo(bool occupied = false, Sides side = White, int index = -1) : occupied(occupied), side(side),
                                                                                     index(index) {}
};

#endif // CHASS_SQUAREINFO_H
#include <algorithm>

#include "enums.h"
#include "matchers.h"
#include "piece.h"

int computeMatcherRank(const Piece &piece) {
    return piece.side == White ? 7 - piece.square.rank : piece.square.rank; // Map tables are upside down
}

ZeroMatcher::ZeroMatcher(Pieces type, const int (&map)[8][8]) {
    this->type = type;
    this->map = &map;
    counter = 0;
}

void ZeroMatcher::add(const Piece &piece) {
    if (piece.kind != type) {
        return;
    }
    int rank = computeMatcherRank(piece);
    int file = piece.square.file;
    counter += (*map)[rank][file];
}

int ZeroMatcher::count() {
    return counter;
}

SingleMatcher::SingleMatcher(Pieces type, const int (&map)[8][8], const int (&promoted)[8][8]) {
    this->type = type;
    this->map = &map;
    this->promoted = &promoted;
    sumPromoted = 0;
    maxDifference = 0;
}

void SingleMatcher::add(const Piece &piece) {
    if (piece.kind != type) {
        return;
    }
    int rank = computeMatcherRank(piece);
    int file = piece.square.file;
    if ((*map)[rank][file] < 0) {
        return;
    }
    sumPromoted += (*promoted)[rank][file];
    int difference = (*promoted)[rank][file] - (*map)[rank][file];
    if (difference > maxDifference) {
        maxDifference = difference;
    }
}

int SingleMatcher::count() {
    return sumPromoted - maxDifference;
}

DoubleMatcher::DoubleMatcher(Pieces type, const int (&map)[8][8], const int (&promoted)[8][8]) {
    this->type = type;
    this->map = &map;
    this->promoted = &promoted;
    sumPromoted = 0;
    totalPieces = 0;
    firstLeftMax = secondLeftMax = firstRightMax = secondRightMax = 0;
    leftIndex = rightIndex = 0;
}

void DoubleMatcher::add(const Piece &piece) {
    if (piece.kind != type) {
        return;
    }
    int rank = computeMatcherRank(piece);
    int leftFile = piece.square.file;
    int rightFile = 7 - piece.square.file;
    int promotedValue = (*promoted)[rank][leftFile];
    sumPromoted += promotedValue;

    int leftDifference = promotedValue - (*map)[rank][leftFile];
    if (leftDifference > firstLeftMax) {
        secondLeftMax = firstLeftMax;
        firstLeftMax = leftDifference;
        leftIndex = totalPieces;
    } else if (leftDifference > secondLeftMax) {
        secondLeftMax = leftDifference;
    }

    int rightDifference = promotedValue - (*map)[rank][rightFile];
    if (rightDifference > firstRightMax) {
        secondRightMax = firstRightMax;
        firstRightMax = rightDifference;
        rightIndex = totalPieces;
    } else if (rightDifference > secondRightMax) {
        secondRightMax = rightDifference;
    }

    ++totalPieces;
}

int DoubleMatcher::count() {
    return sumPromoted - (leftIndex != rightIndex
        ? firstLeftMax + firstRightMax
        : std::max(firstLeftMax + secondRightMax, firstRightMax + secondLeftMax));
}
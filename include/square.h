#ifndef CHASS_SQUARE_H
#define CHASS_SQUARE_H

#include <string>

struct Square {
    int file, rank;

    bool operator ==(const Square &square) const;
    [[nodiscard]] std::string toAlgebraic() const;
    static bool isValidFile(char c);
    static int fileFromAlgebraic(char c);
    static bool isValidRank(char c);
    static int rankFromAlgebraic(char c);
    [[nodiscard]] Square shift(int fileDelta, int rankDelta) const;
    explicit Square(int file = 0, int rank = 0);
};

#endif // CHASS_SQUARE_H
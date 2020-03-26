#include <string>

#include "exceptions.h"
#include "helper.h"
#include "square.h"

bool Square::operator ==(const Square &square) const {
    return square.file == file && square.rank == rank;
}

[[nodiscard]] std::string Square::toAlgebraic() const {
    return Helper::charToString('a' + file) + std::to_string(rank + 1);
}

bool Square::isValidFile(char c) {
    return c >= 'a' && c <= 'h';
}

int Square::fileFromAlgebraic(char c) {
    if (!isValidFile(c)) {
        throw UnknownFile(c);
    }
    return c - 'a';
}

bool Square::isValidRank(char c) {
    return c >= '1' && c <= '8';
}

int Square::rankFromAlgebraic(char c) {
    if (!isValidRank(c)) {
        throw UnknownRank(c);
    }
    return c - '1';
}

[[nodiscard]] Square Square::shift(int fileDelta, int rankDelta) const {
    return Square(file + fileDelta, rank + rankDelta);
}

Square::Square(int file, int rank) : file(file), rank(rank) {}
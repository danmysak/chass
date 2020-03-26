#ifndef CHASS_POSITION_CHAIN_H
#define CHASS_POSITION_CHAIN_H

#include <vector>

#include "move.h"
#include "position.h"

struct PositionChainInfo {
    PackedPosition position;
    Move move;
    int nextInChain;
    PositionChainInfo(const PackedPosition &position, const Move &move,
                      int nextInChain) : position(position), move(move), nextInChain(nextInChain) {}
};

struct PositionChainLevel {
    int startingIndex, length;
    PositionChainLevel(int startingIndex, int length) : startingIndex(startingIndex), length(length) {}
};

class PositionChain {
private:
    std::vector<std::vector<PositionChainInfo>> chain;
    std::vector<PositionChainLevel> levels = {{0, 0}};
public:
    void add(const PackedPosition &position, const Move &move, int nextInChain);
    [[nodiscard]] const PositionChainInfo &get(int index) const;
    void startNextLevel();
    [[nodiscard]] const PositionChainLevel &lastLevel() const;
    [[nodiscard]] const PositionChainLevel &secondLastLevel() const;
    [[nodiscard]] int levelCount() const;
};

#endif // CHASS_POSITION_CHAIN_H
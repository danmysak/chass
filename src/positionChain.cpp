#include "positionChain.h"

constexpr int blockLengthLog = 12;
constexpr int blockLength = 1 << blockLengthLog;
constexpr int mask = blockLength - 1;

void PositionChain::add(const PackedPosition &position, const Move &move, int nextInChain) {
    if (chain.empty() || chain.back().size() == blockLength) {
        chain.emplace_back();
        chain.back().reserve(blockLength);
    }
    chain.back().emplace_back(position, move, nextInChain);
    ++levels.back().length;
}

const PositionChainInfo &PositionChain::get(int index) const {
    return chain[index >> blockLengthLog][index & mask];
}

void PositionChain::startNextLevel() {
    levels.emplace_back(levels.back().startingIndex + levels.back().length, 0);
}

const PositionChainLevel &PositionChain::lastLevel() const {
    return levels.back();
}

const PositionChainLevel &PositionChain::secondLastLevel() const {
    return levels[levels.size() - 2];
}

int PositionChain::levelCount() const {
    return levels.size();
}
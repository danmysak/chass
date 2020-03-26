#include <string>

#include "enums.h"
#include "helper.h"

int Helper::sgn(int a) {
    if (a > 0) return 1;
    if (a < 0) return -1;
    return 0;
}

Sides Helper::opposite(Sides side) {
    return side == White ? Black : White;
}

std::string Helper::sideToString(Sides side, bool capitalize) {
    if (side == White) {
        return capitalize ? "White" : "white";
    } else {
        return capitalize ? "Black" : "black";
    }
}

std::string Helper::castlingSideToString(CastlingSides castlingSide, bool capitalize) {
    if (castlingSide == Kingside) {
        return capitalize ? "Kingside" : "kingside";
    } else {
        return capitalize ? "Queenside" : "queenside";
    }
}

std::string Helper::charToString(int c) {
    std::string s;
    if (c >= 32) {
        s += char(c);
    } else {
        s += "<" + std::to_string(c) + ">";
    }
    return s;
}

bool Helper::canBeSpecialization(Ternary a, Ternary b) {
    return b == Unknown || a == b;
}

bool Helper::canBeSpecialization(bool aLog, int a, bool bLog, int b) {
    return !bLog || (aLog && a == b);
}
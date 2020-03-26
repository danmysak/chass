#ifndef CHASS_HELPER_H
#define CHASS_HELPER_H

#include <string>

#include "enums.h"

class Helper {
public:
    static int sgn(int a);
    static Sides opposite(Sides side);
    static std::string sideToString(Sides side, bool capitalize = false);
    static std::string castlingSideToString(CastlingSides castlingSide, bool capitalize = false);
    static std::string charToString(int c);
    static bool canBeSpecialization(Ternary a, Ternary b);
    static bool canBeSpecialization(bool aLog, int a, bool bLog, int b);
};

#endif // CHASS_HELPER_H
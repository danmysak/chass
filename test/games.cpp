#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "advancer.h"
#include "analyzer.h"
#include "exceptions.h"
#include "FENParser.h"
#include "retractor.h"
#include "validator.h"

bool readMove(const std::string &game, int &cursor, std::string &notation) {
    while (cursor < game.size() && ((game[cursor] >= '0' && game[cursor] <= '9')
                                    || game[cursor] == '.' || game[cursor] == ' ')) {
        ++cursor;
    }
    if (cursor == game.size() || game[cursor] == '{' || game[cursor] == '\r') {
        return false;
    }
    notation.clear();
    while (cursor < game.size() && game[cursor] != ' ' && game[cursor] != '\r') {
        notation += game[cursor];
        ++cursor;
    }
    return true;
}

Position copyOrWeaken(const Position &position, bool weaken) {
    if (weaken) {
        std::string FEN = position.toFENPlacement(true);
        Position weakened = FENParser::parse(FEN);
        Validator::validateAndStrictenUserPosition(weakened); // We need consistent castling and en passant indicators
        return weakened;
    } else {
        return position;
    }
}

bool checkMoveProcessing(const Position &from, const Position &to, const Move &move, bool weaken,
                         void (*enumerate)(const Position &, std::vector<Move> &),
                         void (*perform)(Position &, const Move &), bool onlyComparePlacement = false) {
    Position position = copyOrWeaken(from, weaken);
    std::vector<Move> possibleMoves;
    enumerate(position, possibleMoves);
    bool foundMove = false;
    for (auto &possibleMove : possibleMoves) {
        if (possibleMove.sameAs(move)) {
            foundMove = true;
            break;
        }
    }
    if (!foundMove) {
        return false;
    }
    perform(position, move);
    return onlyComparePlacement ? to.toFENPlacement() == position.toFENPlacement() : to.canBeSpecializationOf(position);
}

bool checkAdvancerRetractorCorrespondence(const Position &nonweakened, bool weaken) {
    Position position = copyOrWeaken(nonweakened, weaken);
    std::vector<Move> advanceMoves;
    Advancer::enumerateMoves(position, advanceMoves);
    for (auto &move : advanceMoves) {
        Position next = position;
        Advancer::advance(next, move);
        if (!checkMoveProcessing(next, position, move, false, Retractor::enumerateMoves, Retractor::retract, weaken)) {
            return false;
        }
    }
    std::vector<Move> retractMoves;
    Retractor::enumerateMoves(position, retractMoves);
    for (auto &move : retractMoves) {
        Position prev = position;
        Retractor::retract(prev, move);
        if (!checkMoveProcessing(prev, position, move, false, Advancer::enumerateMoves, Advancer::advance, weaken)) {
            return false;
        }
    }
    return true;
}

bool checkValidationForFalseNegatives(const Position &nonweakened, bool weaken) {
    Position position = copyOrWeaken(nonweakened, weaken);
    if (!Validator::validate(position)) {
        return false;
    }
    std::vector<Move> advanceMoves;
    Advancer::enumerateMoves(position, advanceMoves);
    for (auto &move : advanceMoves) {
        Position next = position;
        Advancer::advance(next, move);
        if (Validator::validateChecks(next) && !Validator::validate(next)) {
            return false;
        }
    }
    return true;
}

bool process(const std::string &game) {
    Position current = Analyzer::getStartingPosition();
    int cursor = 0;
    std::string notation;
    while (readMove(game, cursor, notation)) {
        try {
            bool check, mate;
            Move move = Analyzer::interpretShortAlgebraic(notation, current, check, mate);
            check = check || mate;
            Position previous = current;
            Advancer::advance(current, move);
            if (mate != Analyzer::isCheckmated(current)) {
                return false;
            }
            if (check != Analyzer::isInCheck(current)) {
                return false;
            }
            if (!checkValidationForFalseNegatives(current, false) || !checkValidationForFalseNegatives(current, true)) {
                return false;
            }
            if (!checkMoveProcessing(current, previous, move, false, Retractor::enumerateMoves, Retractor::retract)
                || !checkMoveProcessing(current, previous, move, true, Retractor::enumerateMoves, Retractor::retract)
                || !checkMoveProcessing(previous, current, move, true, Advancer::enumerateMoves, Advancer::advance)) {
                // Non-weakened forward processing has been implicitly checked via interpretShortAlgebraic
                return false;
            }
            if (!checkAdvancerRetractorCorrespondence(current, false)
                || !checkAdvancerRetractorCorrespondence(current, true)) {
                return false;
            }
        } catch (AlgebraicInterpretationError &e) {
            return false;
        }
    }
    return true;
}

int main() {
    bool passed = true;
    std::ifstream input;
    input.open("data/games.pgn");
    int current = 0;
    while (!input.eof()) {
        std::string line;
        std::getline(input, line);
        if (line.size() < 2 || line[0] != '1' || line[1] != '.') {
            continue;
        }
        if (!process(line)) {
            passed = false;
            break;
        }
        ++current;
        std::cout << "Processed game " + std::to_string(current) << std::endl;
    }
    input.close();
    return passed ? 0 : 1;
}
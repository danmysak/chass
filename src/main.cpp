#include <iostream>
#include <string>
#include <tuple>
#include <unistd.h>
#include <vector>

#include "advancer.h"
#include "analyzer.h"
#include "backtracker.h"
#include "exceptions.h"
#include "FENParser.h"
#include "helper.h"
#include "meeterInTheMiddle.h"
#include "move.h"
#include "progressReporter.h"
#include "validator.h"

constexpr char fullExaminationDepthFlag = 'd';
constexpr char proofExtraDepthFlag = 'e';
constexpr char showProgressFlag = 'r';

void output(const Position &position, const std::vector<Move> &moves, int fullExaminationDepth) {
    std::cout << position.toFENPlacement();
    int totalDepth = moves.size();
    if (totalDepth > 0) { // Otherwise moves[0] is not defined
        int currentMove = position.getFullMoveLog()
                ? position.getFullMoveCounter()
                : -(totalDepth + (moves[0].side == Black ? 1 : 0)) / 2; // So that the input position's move is 0
        Position current = position;
        bool lineBreak = true;
        for (int depth = totalDepth - 1; depth >= 0; --depth) {
            const Move &move = moves[depth];
            if (lineBreak) {
                std::cout << std::endl << currentMove << ".";
                if (move.side == Black) {
                    std::cout << " -";
                }
            }
            Advancer::advance(current, move);
            bool check = Analyzer::isInCheck(current);
            bool mate = depth == 0 && check && Analyzer::isCheckmated(current);
            std::cout << " " << move.toLongAlgebraic(check, mate);
            lineBreak = false;
            if (depth == fullExaminationDepth && depth > 0) {
                std::cout << std::endl << current.toFENPlacement();
                lineBreak = true;
            }
            if (move.side == Black) {
                ++currentMove;
                lineBreak = true;
            }
        }
    }
    std::cout << std::endl << "-----" << std::endl;
}

void progress(const std::vector<std::pair<int, int>> &info) {
    if (info.empty()) {
        std::cerr << "Done.";
    } else if (info.size() == 1 && info[0].second == 0) { // The "starting" signal is sent by the Reporter by convention
        std::cerr << "Starting...";
    } else {
        bool firstOutput = true;
        for (auto &item : info) {
            std::cerr << (firstOutput ? "" : " ") << (item.first + 1) << "/" << item.second;
            // First item is the number of completed steps, but we increment it and show the current step instead
            firstOutput = false;
        }
    }
    std::cerr << std::endl;
}

void error(const std::string &description, const std::string &details = "") {
    std::cerr << description << std::endl;
    if (!details.empty()) {
        std::cerr << "- " << details << std::endl;
    }
}

bool readParams(int argc, char **argv, int &fullExaminationDepth, int &proofExtraDepth, bool &showProgress) {
    fullExaminationDepth = proofExtraDepth = -1;
    showProgress = false;
    std::string issue;
    while (true) {
        std::string description = Helper::charToString(fullExaminationDepthFlag) + ":" +
                                  Helper::charToString(proofExtraDepthFlag) + ":" +
                                  Helper::charToString(showProgressFlag);
        int option = getopt(argc, argv, description.c_str());
        if (option == EOF) {
            break;
        }
        switch (option) {
            case fullExaminationDepthFlag:
            case proofExtraDepthFlag:
                try {
                    int value = std::stoi(optarg);
                    if (value < 0) {
                        issue = "Depth must be non-negative";
                    } else if (option == fullExaminationDepthFlag) {
                        fullExaminationDepth = value;
                    } else {
                        proofExtraDepth = value;
                    }
                } catch (const std::invalid_argument &e) {
                    issue = "Depth must be an integer";
                } catch (const std::out_of_range &e) {
                    issue = "Depth is too large";
                }
                break;
            case showProgressFlag:
                showProgress = true;
                break;
            default:
                issue = "Unknown argument passed";
                break;
        }
    }
    if (issue.empty() && fullExaminationDepth < 0 && proofExtraDepth < 0) {
        issue = "At least one depth parameter must be specified";
    }
    if (issue.empty()) {
        fullExaminationDepth = std::max(0, fullExaminationDepth);
        proofExtraDepth = std::max(0, proofExtraDepth);
        return true;
    } else {
        error(std::string("Valid usage: chass ") +
              "[-" + Helper::charToString(fullExaminationDepthFlag) + " {depth of exhaustive examination}] " +
              "[-" + Helper::charToString(proofExtraDepthFlag) + " {extra proof depth}] " +
              "[-" + Helper::charToString(showProgressFlag) + " (report progress to stderr)]", issue);
        return false;
    }
}

bool readPosition(Position &position) {
    std::string input;
    getline(std::cin, input);
    try {
        position = FENParser::parse(input);
    } catch (const FENParseError &e) {
        error("FEN parsing failed", e.what());
        return false;
    }
    bool valid;
    std::string issue;
    std::tie(valid, issue) = Validator::validateAndStrictenUserPosition(position);
    if (!valid) {
        error("The position is not valid", issue);
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    int fullExaminationDepth, proofExtraDepth;
    bool showProgress;
    if (!readParams(argc, argv, fullExaminationDepth, proofExtraDepth, showProgress)) {
        return 1;
    }
    Position position;
    if (!readPosition(position)) {
        return 1;
    }

    ProgressReporter reporter(showProgress ? progress : nullptr);
    if (proofExtraDepth == 0 && fullExaminationDepth > 1
        && position.getFullMoveLog() && position.getPlyCounter() == fullExaminationDepth + 1) {
        MeeterInTheMiddle meeterInTheMiddle(output, reporter);
        meeterInTheMiddle.search(position, fullExaminationDepth);
    } else {
        Backtracker backtracker(output, reporter);
        backtracker.search(position, fullExaminationDepth, fullExaminationDepth + proofExtraDepth);
    }
}
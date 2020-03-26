#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "backtracker.h"
#include "FENParser.h"
#include "meeterInTheMiddle.h"
#include "progressReporter.h"
#include "validator.h"

int counter;

void output(const Position &position, const std::vector<Move> &moves, int fullExaminationDepth) {
    ++counter;
}

bool process(const Position &position, int fullExaminationDepth, int proofExtraDepth, int answerCount) {
    counter = 0;
    ProgressReporter reporter(nullptr);
    if (proofExtraDepth == 0 && fullExaminationDepth > 1
        && position.getFullMoveLog() && position.getPlyCounter() == fullExaminationDepth + 1) {
        MeeterInTheMiddle meeterInTheMiddle(output, reporter);
        meeterInTheMiddle.search(position, fullExaminationDepth);
    } else {
        Backtracker backtracker(output, reporter);
        backtracker.search(position, fullExaminationDepth, fullExaminationDepth + proofExtraDepth);
    }
    return counter == answerCount;
}

int main() {
    bool passed = true;
    std::ifstream input;
    input.open("data/problems.txt");
    int current = 0;
    while (!input.eof()) {
        std::string line, params, answers, separator;
        std::getline(input, line);
        Position position = FENParser::parse(line);
        Validator::validateAndStrictenUserPosition(position);
        std::getline(input, params);
        int fullExaminationDepth, proofExtraDepth;
        std::istringstream(params) >> fullExaminationDepth >> proofExtraDepth;
        std::getline(input, answers);
        int answerCount;
        std::istringstream(answers) >> answerCount;
        std::getline(input, separator);
        if (!process(position, fullExaminationDepth, proofExtraDepth, answerCount)) {
            passed = false;
            break;
        }
        ++current;
        std::cout << "Processed problem " + std::to_string(current) << std::endl;
    }
    input.close();
    return passed ? 0 : 1;
}
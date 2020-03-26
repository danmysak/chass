cmake_minimum_required(VERSION 3.10)
project(chass)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(algo
        include/advancer.h include/analyzer.h include/backtracker.h include/FENParser.h include/helper.h
        include/matchers.h include/meeterInTheMiddle.h include/move.h include/piece.h include/position.h
        include/positionChain.h include/progressReporter.h include/retractor.h include/searcher.h include/square.h
        include/validator.h
        src/advancer.cpp src/analyzer.cpp src/backtracker.cpp src/FENParser.cpp src/helper.cpp
        src/matchers.cpp src/meeterInTheMiddle.cpp src/move.cpp src/piece.cpp src/position.cpp
        src/positionChain.cpp src/progressReporter.cpp src/retractor.cpp src/searcher.cpp src/square.cpp
        src/validator.cpp)

add_subdirectory(src)

enable_testing()
add_subdirectory(test)
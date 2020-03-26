add_executable(test_games games.cpp)
target_link_libraries(test_games algo)
add_test(NAME games COMMAND test_games WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_problems problems.cpp)
target_link_libraries(test_problems algo)
add_test(NAME problems COMMAND test_problems WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
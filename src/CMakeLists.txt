add_executable(chass main.cpp)
target_link_libraries(chass algo)
install(TARGETS chass RUNTIME DESTINATION bin)
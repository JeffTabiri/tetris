cmake_minimum_required(VERSION 3.20)
project(tetris)

find_package(SDL3 REQUIRED)

add_executable(tetris src/main.cpp src/game.cpp)

target_link_libraries(tetris PRIVATE SDL3::SDL3)
target_include_directories(tetris PRIVATE include)
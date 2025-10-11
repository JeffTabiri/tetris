cmake_minimum_required(VERSION 3.16)
project(viola)

find_package(SDL3 REQUIRED)
add_executable(viola main.cpp)

target_link_libraries(viola SDL3::SDL3)

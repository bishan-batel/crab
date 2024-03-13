cmake_minimum_required(VERSION 3.28)
project(crabpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_FLAGS
        -Wall
        -Wextra
        -Wfloat-equal
        -Wformat=2
)
set(CMAKE_CXX_STANDARD 23)

add_executable(crabpp main.cpp)

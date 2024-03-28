cmake_minimum_required(VERSION 3.28)
project(crabpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_FLAGS -Wall)
set(CMAKE_CXX_STANDARD 23)

include_directories(src)

add_executable(crabpp src/main.cpp
        src/preamble.hpp
        src/ref.hpp
        src/range.hpp
        src/option.hpp
        src/box.hpp
)

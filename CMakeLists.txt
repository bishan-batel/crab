cmake_minimum_required(VERSION 3.2)
project(crabpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS -Wall)

set(CRABPP_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/inc)

#add_library(${PROJECT_NAME} SHARED inc/preamble.hpp inc/option.hpp inc/box.hpp
#                                   inc/range.hpp inc/ref.hpp)

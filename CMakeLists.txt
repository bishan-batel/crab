cmake_minimum_required(VERSION 3.5)

project(crab VERSION 2.0 DESCRIPTION "crab description")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

set(CPM_SOURCE_CACHE_DEFAULT ON)

# CPM should find local packages before trying to download

# CPM
include(cmake/CPM.cmake)

option(CRAB_USE_FMT "Should crab use / require FMT to format" OFF)

set(CPM_USE_LOCAL_PACKAGES ON)

if(CRAB_USE_FMT)
        CPMAddPackage("gh:fmtlib/fmt#7.1.3")
endif()

# Setting up library & install
include(GNUInstallDirs)

set(CRAB_HEADERS "")
file(GLOB CRAB_HEADERS "include/crab/*")
add_library(crab STATIC ${CRAB_HEADERS})

# Platform Specific flags
if (APPLE)
        target_compile_definitions(crab PUBLIC "_OSX=1")
elseif (LINUX)
        target_compile_definitions(crab PUBLIC "_LINUX=1")
elseif(BSD)
        target_compile_definitions(crab PUBLIC "_BSD=1")
elseif(WIN32)
        target_compile_definitions(crab PUBLIC "_WIN32=1")
endif()

if(UNIX)
        target_compile_definitions(crab PUBLIC "_UNIX=1")
endif()

# Public API
set_target_properties(crab PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        PUBLIC_HEADER "include/crab/all.hpp"
        LINKER_LANGUAGE CXX
)

target_include_directories(crab PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(crab PUBLIC
        "CRAB_DEBUG=$<IF:$<CONFIG:Debug>,1,0>"
        "CRAB_RELEASE=$<IF:$<CONFIG:RELEASE>,1,0>")


if(CRAB_USE_FMT)
        target_compile_definitions(crab PUBLIC "CRAB_FMT_USAGE=1")
        target_link_libraries(crab PUBLIC fmt)
endif()


install(TARGETS crab DESTINATION lib)
install(DIRECTORY include/crab/ DESTINATION include/crab)

option(CRAB_TESTS "Whether to compile / include crab tests" OFF)

if(CRAB_TESTS)
        add_subdirectory(test)
endif()

cmake_minimum_required(VERSION 3.5)

project(crab VERSION 1.0.1 DESCRIPTION "crab description")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

# CPM should find local packages before trying to download

# CPM
include(cmake/CPM.cmake)

set(CPM_USE_LOCAL_PACKAGES ON)
CPMAddPackage("gh:fmtlib/fmt#7.1.3")

# Setting up library & install
include(GNUInstallDirs)

add_library(${PROJECT_NAME} SHARED
        include/crab/type_traits.hpp
        include/crab/debug.hpp
        include/crab/option.hpp
        include/crab/error.hpp
        include/crab/result.hpp
        include/crab/ref.hpp
        include/crab/box.hpp
        include/crab/rc.hpp
)

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
        PUBLIC_HEADER "include/preamble.hpp"
        LINKER_LANGUAGE CXX
)

target_include_directories(crab PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(crab
        PUBLIC
        "_DEBUG=$<IF:$<CONFIG:Debug>,1,0>"
        "_RELEASE=$<IF:$<CONFIG:Debug>,0,1>"
        "_NDEBUG=$<IF:$<CONFIG:Debug>,0,1>"
)

target_link_libraries(crab PUBLIC fmt)

install(TARGETS crab DESTINATION lib)
install(DIRECTORY include/crab/ DESTINATION include/crab)

option(CRAB_TESTS OFF "Whether to compile / include crab tests")

if(CRAB_TESTS)
        add_subdirectory(test)
endif()

cmake_minimum_required(VERSION 3.31)
project(crab VERSION 2.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

project(${PROJECT_NAME} VERSION 1.0.1 DESCRIPTION "${PROJECT_NAME} description")

# CPM
include(cmake/CPM.cmake)

CPMAddPackage("gh:fmtlib/fmt#7.1.3")

# Setting up library & install
include(GNUInstallDirs)

add_library(${PROJECT_NAME} SHARED
        include/box.hpp
        include/box.hpp
        include/option.hpp
        include/preamble.hpp
        include/rc.hpp
        include/ref.hpp
        include/crab/debug.hpp
        include/result.hpp
        include/result.hpp
        include/crab/type_traits.hpp
        include/error.hpp
)

# Public API
set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        PUBLIC_HEADER "include/preamble.hpp"
        LINKER_LANGUAGE CXX)

configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)

target_include_directories(${PROJECT_NAME} PUBLIC include)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)


target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        "_DEBUG=$<IF:$<CONFIG:Debug>,1,0>"
        "_RELEASE=$<IF:$<CONFIG:Debug>,0,1>"
        "_NDEBUG=$<IF:$<CONFIG:Debug>,0,1>"
)

target_link_libraries(${PROJECT_NAME} PUBLIC fmt)

option(CRAB_TESTS OFF "Whether to compile / include crab tests")

if(CRAB_TESTS)
        add_subdirectory(test)
endif()

if (APPLE)
        target_compile_definitions(${PROJECT_NAME} PUBLIC "_OSX=1")
elseif (LINUX)
        target_compile_definitions(${PROJECT_NAME} PUBLIC "_LINUX=1")
elseif(BSD)
        target_compile_definitions(${PROJECT_NAME} PUBLIC "_BSD=1")
elseif(WIN32)
        target_compile_definitions(${PROJECT_NAME} PUBLIC "_WIN32=1")
elseif(UNIX)
        target_compile_definitions(${PROJECT_NAME} PUBLIC "_UNIX=1")
endif()

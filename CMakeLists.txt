cmake_minimum_required(VERSION 3.5)
project(crab VERSION 0.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -fexceptions")

cmake_minimum_required(VERSION 3.9)
project(${PROJECT_NAME} VERSION 1.0.1 DESCRIPTION "${PROJECT_NAME} description")

# CPM
include(cmake/CPM.cmake)

# Setting up library & install
include(GNUInstallDirs)

add_library(${PROJECT_NAME} SHARED
        src/debug.cpp
        include/box.hpp
        include/box.hpp
        include/option.hpp
        include/preamble.hpp
        include/range.hpp
        include/rc.hpp
        include/range.hpp
        include/ref.hpp
        include/debug.hpp
        include/result.hpp
        include/result.hpp
        include/pattern_match.hpp
        include/crab_type_traits.hpp
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
        PRIVATE
        "DEBUG=$<IF:$<CONFIG:Debug>,1,0>"
        "RELEASE=$<IF:$<CONFIG:Debug>,0,1>"
)

add_subdirectory(test)

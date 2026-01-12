cmake_minimum_required(VERSION 3.5..3.10)

# grab version from file
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" CRAB_VERSION_STRINGS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(crab VERSION "${CRAB_VERSION_STRINGS}" LANGUAGES CXX)

# Hack for fixing intellisense on nixos/apple
if(${CMAKE_EXPORT_COMPILE_COMMANDS})
        set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
        set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

#===============================================================================
#                                     Options
#===============================================================================

option(CRAB_TESTS "Whether to compile / include crab tests, " OFF)
option(CPM_USE_LOCAL_PACKAGES "Should CPM try to find local packages" ON)
option(CPM_SOURCE_CACHE_DEFAULT "Should CPM cache the source for dependencies?" ON)

if (CRAB_USE_STD_FORMAT AND CRAB_USE_FMT)
        message(FATAL_ERROR "CRAB_USE_STD_FORMAT and CRAB_USE_FMT are mutually exclusive, choose whether crab should use std::format vs fmt::format")
endif()


#===============================================================================
#                                   Dependencies
#===============================================================================

# dependency management (goes unused if no tests are enable & fmtlib is disabled)
include(cmake/CPM.cmake)

# cmake helpers for installation of the library 
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# fmtlib is optional
message(STATUS "Adding fmtlib")
set(FMT_INSTALL ON)

CPMAddPackage(
        NAME fmt
        GITHUB_REPOSITORY "fmtlib/fmt"
        GIT_TAG "9.0.0"
        VERSION_MIN "7.1.3"
)

#===============================================================================
#                                     Library
#===============================================================================
set(CRAB_CONFIG_HEADER "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/config.hpp")

# configuration file containing versioning & platform-specific information
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/config.hpp.in"
        "${CRAB_CONFIG_HEADER}"
        @ONLY
)

# interface library
set(CRAB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}")
add_library(${PROJECT_NAME} INTERFACE
        ./include/crab/boxed/Box.hpp 
        ./include/crab/casts.hpp 

        ./include/crab/cmp/max.hpp 
        ./include/crab/cmp/min.hpp 

        ./include/crab/core.hpp 
        ./include/crab/fmt.hpp 
        ./include/crab/fn.hpp 
        ./include/crab/hash.hpp 

        ./include/crab/mem/address_of.hpp 
        ./include/crab/mem/copy.hpp 
        ./include/crab/mem/forward.hpp 
        ./include/crab/mem/mem.hpp 
        ./include/crab/mem/move.hpp 
        ./include/crab/mem/replace.hpp 
        ./include/crab/mem/size_of.hpp 
        ./include/crab/mem/swap.hpp 
        ./include/crab/mem/take.hpp 

        ./include/crab/opt/Option.hpp 
        ./include/crab/opt/boolean_constructs.hpp 
        ./include/crab/opt/fallible.hpp 
        ./include/crab/opt/impl/GenericStorage.hpp 
        ./include/crab/opt/impl/RefStorage.hpp 
        ./include/crab/opt/impl/fallible.hpp 
        ./include/crab/opt/none.hpp 
        ./include/crab/opt/opt.hpp 
        ./include/crab/opt/some.hpp 

        ./include/crab/preamble.hpp 

        ./include/crab/range.hpp 

        ./include/crab/rc/impl/Counter.hpp 
        ./include/crab/rc/impl/RcBase.hpp
        ./include/crab/rc/impl/RcStorage.hpp
        ./include/crab/rc/Rc.hpp 

        ./include/crab/assertion/todo.hpp 

        ./include/crab/core/unreachable.hpp 
        ./include/crab/core/cases.hpp
        ./include/crab/core/discard.hpp

        ./include/crab/ref.hpp 
        ./include/crab/result.hpp 
        ./include/crab/type_traits.hpp
)

# crab only supports C++20 and above
# target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

target_link_libraries(${PROJECT_NAME} INTERFACE fmt)

target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

#===============================================================================
#                           Public API / Installation
#===============================================================================

set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        LINKER_LANGUAGE CXX
)

set(conf_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

# cmake target information (currently only one)
install(TARGETS "${PROJECT_NAME}" EXPORT "${PROJECT_NAME}Targets")

install(
        EXPORT "${PROJECT_NAME}Targets"
        FILE "${PROJECT_NAME}Targets.cmake"
        NAMESPACE "${PROJECT_NAME}::"
        DESTINATION "${conf_install_dir}"
)

# install the library headers
install(
        DIRECTORY "${CRAB_INCLUDE_DIR}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "*.in" EXCLUDE # exclude the config.hpp.in template
)

# install the platform/version specific config.hpp
install(
        FILES "${CRAB_CONFIG_HEADER}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)


# Package Config files for use with find_package

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${conf_install_dir}"
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
         VERSION ${PROJECT_VERSION}
         COMPATIBILITY SameMajorVersion
)

install(
        FILES   "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
        DESTINATION "${conf_install_dir}"
)

# Tests

if(CRAB_TESTS)
        enable_testing()
        add_subdirectory(tests)
endif()

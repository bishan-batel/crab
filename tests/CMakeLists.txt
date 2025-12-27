CPMAddPackage(
        NAME Catch2
        GITHUB_REPOSITORY "catchorg/Catch2"
        GIT_TAG "v3.5.4"
        VERSION_MIN "3.0.0"
)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()

# Tests
add_executable(crab::tests
        box.cpp
        result.cpp
        option.cpp
        rc.cpp
        pattern.cpp
        monadic.cpp
        fn.cpp
        ref.cpp
        fallible.cpp
)

target_link_libraries(crab::tests PRIVATE crab Catch2::Catch2WithMain)

if (MSVC)
    target_compile_options(crab::tests PUBLIC 
        /W3 /WX
    )
else()
    target_compile_options(crab::tests PUBLIC 
        -Wall 
        -Werror 
        -Wpedantic 
        -Wextra 
        -pedantic
        -O3
        -fsanitize-trap=undefined
    )
endif()

target_compile_definitions(crab::tests PUBLIC "CATCH_CONFIG_DISABLE_EXCEPTIONS=$<IF:$<CONFIG:Debug>,0,1>")

include(CTest)
include(Catch)
catch_discover_tests(crab::tests)

CPMAddPackage("gh:catchorg/Catch2@3.5.4")

option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()

# Tests
add_executable(crab-tests
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
target_link_libraries(crab-tests PRIVATE Catch2::Catch2WithMain crab)
target_compile_options(crab-tests PRIVATE -Wall -Werror -Wpedantic -Wextra -O1)

target_compile_definitions(crab-tests
        PRIVATE "DEBUG=$<IF:$<CONFIG:Debug>,1,0>")

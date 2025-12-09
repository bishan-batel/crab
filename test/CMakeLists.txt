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

target_link_libraries(crab-tests PRIVATE crab Catch2::Catch2WithMain)
target_compile_options(crab-tests PUBLIC 
    -Wall 
    -Werror 
    -Wpedantic 
    -Wextra 
    -pedantic
    -O3
    -fsanitize-trap=undefined
)

CPMAddPackage("gh:catchorg/Catch2@3.6.0")

# Tests
add_executable(crab-tests
        box.cpp
        result.cpp
        pattern.cpp
        rc.cpp
)

target_link_libraries(crab-tests PRIVATE Catch2::Catch2WithMain crab)

target_compile_definitions(crab-tests
        PRIVATE "DEBUG=$<IF:$<CONFIG:Debug>,1,0>")

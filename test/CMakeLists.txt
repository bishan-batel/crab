CPMAddPackage("gh:catchorg/Catch2@3.5.4")

# Tests
add_executable(tests
        box.cpp
        result.cpp
        pattern.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain crab)

target_compile_definitions(tests
        PRIVATE "DEBUG=$<IF:$<CONFIG:Debug>,1,0>")

#include <preamble.hpp>
#include <option.hpp>
#include <catch2/catch_test_macros.hpp>

auto non_zero(u8 x) -> Option<u8> {
  return crab::some(x).filter([](u8 x) { return x != 0; });
}

TEST_CASE("Fallible (Option)") {
  SECTION("normal use") {
    REQUIRE(non_zero(0).is_none());
    REQUIRE(non_zero(1).is_some());

    auto tuple = crab::fallible( //
      []() { return 0; },
      []() { return non_zero(1); }
    );

    CHECK_NOTHROW(std::get<0>(tuple.get_unchecked()) == 0);
    CHECK_NOTHROW(std::get<1>(tuple.get_unchecked()) == 1);

    CHECK(crab::fallible( //
            []() { return 0; },
            []() { return non_zero(0); }
    ).is_none());

    CHECK(crab::fallible( //
            []() { return non_zero(0); },
            []() { return 0; }
    ).is_none());
  }
}

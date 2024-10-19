#include "option.hpp"
#include <catch2/catch_test_macros.hpp>
#include "preamble.hpp"

TEST_CASE("Option", "[option]") {
  Option<i32> a = crab::some(52);

  i32 took;
  REQUIRE_NOTHROW(took = a.take_unchecked());

  REQUIRE(took == 52);

  REQUIRE_THROWS(a.take_unchecked());

  a = crab::some(42);
  REQUIRE(a.take_unchecked() == 42);

  REQUIRE_THROWS(a.take_unchecked());

  a = crab::some(42);

  SECTION("Nested Options") {
    Option<i32> nested = crab::some(10);

    REQUIRE(nested.map([](auto x) { return 2 * x; }).take_unchecked() == 20);
    REQUIRE(nested.map([](auto x) { return x; }).is_none());

    nested = 420;

    REQUIRE(nested.flat_map([](auto x) { return crab::some(x); }).take_unchecked() == 420);
    REQUIRE(crab::some(crab::some(420)).flatten().take_unchecked() == 420);
  }

  SECTION("Fallible Options") {
    SECTION("All Some crab::fallible") {
      bool first = false, second = false;

      Option<std::tuple<i32, i32>> a = crab::fallible(
          [&]() {
            first = true;
            return 10;
          },
          [&]() {
            second = true;
            return 22;
          });

      REQUIRE((first and second));
      REQUIRE(a.is_some());

      const auto [num1, num2] = a.take_unchecked();
      REQUIRE(num1 == 10);
      REQUIRE(num2 == 22);
    }

    SECTION("Some None crab::fallible") {
      bool first = false, second = false;

      Option<std::tuple<i32, i32>> a = crab::fallible(
          [&]() {
            first = true;
            return 420;
          },
          [&]() -> Option<i32> {
            second = true;
            return crab::none;
          });

      REQUIRE((first and second));
      REQUIRE(a.is_none());

      first = false;
      second = false;

      a = crab::fallible(
          [&]() -> Option<i32> {
            first = true;
            return crab::none;
          },
          [&]() -> Option<i32> {
            second = true;
            return crab::none;
          });

      REQUIRE((first and not second));
      REQUIRE(a.is_none());
    }
  }
}

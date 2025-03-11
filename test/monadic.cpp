#include <catch2/catch_test_macros.hpp>
#include <preamble.hpp>
#include "option.hpp"
#include "test_types.hpp"
#include <crab/fn.hpp>

consteval auto consteval_test() -> void {
  Option<i32> number = crab::unless(false, []() { return 2; });
  number = crab::unless(true, []() { return 2; });

  std::ignore = number.filter([](i32 x) { return x % 2 == 0; });

  Option<MoveOnly> moved{MoveOnly{"value"}};
  std::ignore = std::move(moved).filter(crab::fn::constant(true));
}

TEST_CASE("Monadic Operations (Option)") {
  SECTION("filter") {
    constexpr auto is_even = [](auto x) { return x % 2 == 0; };

    SECTION("copy constructible") {
      for (usize i = 0; i < 10; i++) {
        Option<usize> val = crab::some(i).filter(is_even);

        REQUIRE(val.is_some() == is_even(i));
        REQUIRE(val.is_none() == not is_even(i));
        if (val.is_some()) {
          REQUIRE_NOTHROW(val.get_unchecked() == i);
        }
      }

      // indempotent behavior
      for (usize i = 0; i < 10; i++) {
        Option<usize> val = crab::some(i) //
                              .filter(is_even)
                              .filter(is_even)
                              .filter(is_even)
                              .filter(is_even);

        REQUIRE(val.is_some() == is_even(i));
        if (val.is_some()) {
          REQUIRE_NOTHROW(val.get_unchecked() == i);
        }
      }
    }

    SECTION("move") {
      Option<MoveOnly> value{MoveOnly{"message"}};

      constexpr auto not_empty = [](const MoveOnly& x) {
        return not x.get_name().empty();
      };

      Option<MoveOnly> moved = std::move(value).filter(not_empty);
      REQUIRE(value.is_none());
      REQUIRE_NOTHROW(moved.get_unchecked().get_name() == "message");

      REQUIRE(std::move(value).filter(crab::fn::constant(true)).is_none());
      REQUIRE(std::move(value).filter(crab::fn::constant(false)).is_none());

      // compilation test
      // in gross lambda to force the static assert to happen in a
      // templated context
      [](const auto& ty) {
        STATIC_REQUIRE(not requires {
          ty.filter([](const auto&) { return true; });
        });
      }(moved);
    }
  }

  SECTION("crab::then") {
    SECTION("copyable") {
      Option<i32> number = crab::then(true, []() { return 2; });

      REQUIRE_NOTHROW(number.is_some() and number.get_unchecked() == 2);

      number = crab::then(false, []() { return 2; });
      REQUIRE(number.is_none());
    }

    SECTION("move-only") {
      Option<MoveOnly> number =
        crab::then(true, []() { return MoveOnly{"test"}; });

      REQUIRE_NOTHROW(
        number.is_some() and number.get_unchecked().get_name() == "test"
      );

      number = crab::then(false, []() { return MoveOnly{"test"}; });
      REQUIRE(number.is_none());
    }
  }

  SECTION("crab::unless") {
    SECTION("copyable") {
      Option<i32> number = crab::unless(false, []() { return 2; });
      REQUIRE_NOTHROW(number.is_some() and number.get_unchecked() == 2);

      number = crab::unless(true, []() { return 2; });
      REQUIRE(number.is_none());
    }

    SECTION("move-only") {
      Option<MoveOnly> number =
        crab::unless(false, []() { return MoveOnly{"test"}; });

      REQUIRE_NOTHROW(
        number.is_some() and number.get_unchecked().get_name() == "test"
      );

      number = crab::unless(true, []() { return MoveOnly{"test"}; });
      REQUIRE(number.is_none());
    }
  }

}

#include <algorithm>
#include <option.hpp>
#include <catch2/catch_test_macros.hpp>
#include <functional>
#include "box.hpp"
#include "test_types.hpp"
#include <preamble.hpp>
#include <ref.hpp>
#include <crab/fn.hpp>
#include <range.hpp>

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
    REQUIRE(nested.map(crab::fn::identity).is_some());

    nested = 420;

    REQUIRE(
      nested.flat_map([](auto x) { return crab::some(x); }).take_unchecked()
      == 420
    );
    REQUIRE(crab::some(crab::some(420)).flatten().take_unchecked() == 420);
  }

  SECTION("Fallible Options") {
    SECTION("All Some crab::fallible") {
      bool first = false, second = false;

      Option<Tuple<i32, i32>> a = crab::fallible(
        [&]() {
          first = true;
          return 10;
        },
        [&]() {
          second = true;
          return 22;
        }
      );

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
        }
      );

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
        }
      );

      REQUIRE((first and not second));
      REQUIRE(a.is_none());
    }
  }

  SECTION("move_only") {

    Option<Box<i32>> a{crab::make_box<i32>(10)};

    Option<i32> b = std::move(a).map([](i32 x) { return x * 2; });
    REQUIRE(a.is_none());
    REQUIRE(b.is_some());

    if (Option<bool> opt{true}) {
      REQUIRE_NOTHROW(opt.get_unchecked());
    }
  }

  SECTION("bool conversion") {
    REQUIRE(Option<i32>{0});
    REQUIRE_FALSE(Option<i32>{});

    REQUIRE_FALSE(not crab::some(0));
    REQUIRE(not Option<i32>{});
  }

  SECTION("Comparisons") {

    for (usize i = 0; i < 10; i++) {

      ex::test_values(
        [](auto&& x) {
          Option val{x};

          REQUIRE(val == crab::some(x));

          const auto cmp_none = [&]() {
            REQUIRE(val > crab::none);
            REQUIRE(val < crab::none);
            REQUIRE(val >= crab::none);
            REQUIRE(val <= crab::none);
          };

          cmp_none();
          val = crab::none;
          cmp_none();

          REQUIRE(val == crab::none);
        },
        i,
        static_cast<f32>(i),
        static_cast<u16>(i),
        static_cast<f64>(i),
        static_cast<u8>(i)
      );
    }
  }

  SECTION("Hash") {
    Set<Option<usize>> numbers{};

    for (const usize i: crab::range(100)) {
      numbers.emplace(i);
    }
    for (const usize i: crab::range(100)) {
      REQUIRE(numbers.contains(i));
    }
    REQUIRE_FALSE(numbers.contains(crab::none));
  }
}

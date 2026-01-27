#include <crab/preamble.hpp>

#include <crab/opt/opt.hpp>

#include <catch2/catch_test_macros.hpp>
#include "crab/fn/identity.hpp"

[[nodiscard]] auto non_zero(u8 x) -> Option<u8> {
  return crab::some(x).filter([](u8 x) { return x != 0; });
}

TEST_CASE("Fallible (Option)") {
  REQUIRE(non_zero(0).is_none());
  REQUIRE(non_zero(1).is_some());

  SECTION("normal use") {
    SECTION("Success Cases") {

      {
        Option<Tuple<i32, u8>> tuple{
          crab::fallible([]() { return 0; }, []() { return non_zero(1); }),
        };

        REQUIRE(tuple.is_some());
        if (tuple) {
          auto& [a, b] = tuple.get_unchecked();
          CHECK(a == 0);
          CHECK(b == 1);
        }

        tuple = crab::fallible( //
          crab::fn::constant(0),
          []() { return non_zero(1); }
        );

        REQUIRE(tuple.is_some());
        if (tuple) {
          auto& [a, b] = tuple.get_unchecked();
          CHECK(a == 0);
          CHECK(b == 1);
        }
      }

      {
        auto tuple = crab::fallible( //
          []() { return non_zero(1); },
          []() { return 0; }
        );

        REQUIRE(tuple);
        {
          auto& [b, a] = tuple.get_unchecked();
          CHECK(a == 0);
          CHECK(b == 1);
        }

        tuple = crab::fallible( //
          []() { return non_zero(1); },
          crab::fn::constant(0)
        );

        REQUIRE(tuple);
        auto& [b, a] = tuple.get_unchecked();
        CHECK(a == 0);
        CHECK(b == 1);
      }

      SECTION("raw non-optional value") {
        auto tuple = crab::fallible( //
          []() { return non_zero(1); },
          0,
          42,
          2
        );

        REQUIRE(tuple);
        auto& [b, a, c, d] = tuple.get_unchecked();
        CHECK(a == 0);
        CHECK(b == 1);
        CHECK(c == 42);
        CHECK(d == 2);
      }

      SECTION("raw optional value") {
        Option<Tuple<u8, i32>> tuple = crab::fallible(non_zero(1), 0);

        CHECK(tuple);
        if (tuple) {
          auto& [b, a] = tuple.get_unchecked();
          CHECK(a == 0);
          CHECK(b == 1);
        }
      }
    }

    SECTION("Failure Cases") {
      REQUIRE_FALSE(crab::fallible(crab::fn::constant(0), []() { return non_zero(0); }));

      REQUIRE_FALSE( //
        crab::fallible(Option<i32>{}, []() { return non_zero(0); })
      );

      REQUIRE(crab::fallible(crab::none));

      REQUIRE_FALSE( //
        crab::fallible(Option<i32>{0}, []() { return non_zero(0); })
      );

      SECTION("Short Circuiting") {
        usize a = 0;

        CHECK_FALSE(
          crab::fallible(
            [&]() {
              a++;
              return crab::some(0);
            },
            [&]() {
              a++;
              return crab::some("hi");
            },
            [&]() -> Option<i32> {
              a++;
              return crab::none;
            },
            [&]() -> Option<i32> {
              a++;
              return 10;
            }
          )
        );
        REQUIRE(a == 3);
      }
    }
  }
}

//
// Created by bishan_ on 4/24/24.
//

#include <catch2/catch_test_macros.hpp>
#include <pattern_match.hpp>

class TestError final : public crab::Error {
  String huh;

public:
  explicit TestError(const String &huh) : huh(huh) {}

  String what() const override { return huh; }
};

TEST_CASE("Pattern Matchin", "[pattern]") {
  SECTION("Option") {
    Option<i32> opt = crab::some(10);

    bool state = false;

    SECTION("if_some") {
      REQUIRE_NOTHROW(opt.get_unchecked());

      REQUIRE_NOTHROW(
        crab::if_some(
          std::move(opt),
          [&]([[maybe_unused]] auto some) {
          state = true;
          }
        )
      );

      REQUIRE(state);
      REQUIRE(opt.is_none());
      REQUIRE_THROWS(opt.get_unchecked());

      REQUIRE_NOTHROW(
        crab::if_some(
          std::move(opt),
          [&]([[maybe_unused]] auto some) { state = true; }
        )
      );

      REQUIRE_NOTHROW(
        crab::if_some(
          std::move(opt),
          [&]([[maybe_unused]] auto some) {}
        ).or_else([&] { state = false; })
      );

      REQUIRE_FALSE(state);
    }
    SECTION("if_none") {
      state = false;

      REQUIRE_NOTHROW(
        crab::if_none(
          std::move(opt),
          [&]{ state = true;}
        )
      );

      opt = 10;

      REQUIRE_NOTHROW(
        crab::if_none(
          std::move(opt),
          [&]{ state = false;}
        ).or_else([&]([[maybe_unused]] i32 _) {
          state = true;
          })
      );

      REQUIRE(state);
    }
  }

  SECTION("Result") {
    Result<i32, TestError> result = crab::ok(42);

    i32 v = 0;

    REQUIRE_NOTHROW(
      if_ok(
        std::move(result),
        [&](const auto value) {
        v = value;
        }
      )
    );

    REQUIRE(v == 42);

    REQUIRE_THROWS(result.ensure_valid());
    REQUIRE_THROWS(result.take_unchecked());
    REQUIRE_THROWS(result.take_err_unchecked());

    result = err(TestError{"huh"});

    REQUIRE_THROWS(result.get_unchecked());
    REQUIRE_NOTHROW(result.get_err_unchecked());

    v = 0;

    REQUIRE_NOTHROW(
      if_ok(
        std::move(result),
        [&](const auto value) {
        v = value;
        }
      )
    );

    REQUIRE(v == 0);

    result = err(TestError{"huh"});
    REQUIRE_NOTHROW(
      if_err(
        std::move(result),
        [&]([[maybe_unused]] TestError value) {
        v = 420;
        }
      )
    );

    REQUIRE(v == 420);
    REQUIRE_THROWS(result.ensure_valid());
    REQUIRE_THROWS(result.get_unchecked());
    REQUIRE_THROWS(result.get_err_unchecked());

    v = 0;

    // result = err(TestError{"huh"});
    result = crab::ok(69);

    REQUIRE_NOTHROW(
      if_err(
        std::move(result),
        [&]([[maybe_unused]] TestError value) { v = 420; }
      ).or_else([&] (const i32 value) { v = value; } )
    );

    REQUIRE(v == 69);
  }
}

//
// Created by bishan_ on 4/23/24.
//

#include <catch2/catch_test_macros.hpp>

#include <result.hpp>
#include <pattern_match.hpp>

#include "ref.hpp"

class Error final : public crab::Error {
public:
  [[nodiscard]] auto operator==(const Error &) const -> bool { return true; }

  String what() const override {
    return "huh";
  }
};

TEST_CASE("Result", "[result]") {
  SECTION("Ok Values") {
    Result<u32, Error> result{10};

    REQUIRE(result.is_ok());
    REQUIRE_FALSE(result.is_err());

    REQUIRE(result.get_unchecked() == 10);
    REQUIRE(result.take_unchecked() == 10);

    REQUIRE_THROWS(result.get_unchecked());
    REQUIRE_THROWS(result.get_err_unchecked());
    REQUIRE_THROWS(result.take_unchecked());
    REQUIRE_THROWS(result.take_err_unchecked());

    result = err(Error{});

    REQUIRE(result.is_err());
    REQUIRE_FALSE(result.is_ok());
    REQUIRE_THROWS(result.get_unchecked());
    REQUIRE_NOTHROW(result.get_err_unchecked());

    Error err;
    REQUIRE_NOTHROW(result.ensure_valid());
    REQUIRE_NOTHROW(err = crab::unwrap_err(std::move(result)));

    REQUIRE_THROWS(result.ensure_valid());
    REQUIRE_THROWS(crab::unwrap_err(std::move(result)));
    REQUIRE_THROWS(crab::unwrap(std::move(result)));

    result = crab::ok<u32>(42);
    REQUIRE_NOTHROW(result.ensure_valid());

    REQUIRE(Result<unit, Error>{unit{}}.is_ok());
  }

  SECTION("Reference Pattern Matching") {
    Result<i32, Error> result{10};

    i32 v = 0;
    REQUIRE_NOTHROW(
      if_ok(result, [&](const i32 value) {
        v = value;
        })
    );

    REQUIRE_NOTHROW(
      if_err(result, [&](const Error&) { v = 51358; })
    );

    REQUIRE(v != 51358);

    result = err(Error{});

    REQUIRE_NOTHROW(
      if_err(result, [&](const Error&) { v = 51358; })
    );

    REQUIRE(v == 51358);
  }

  SECTION("std::visit") {
    Result<i32, Error> huh{10};
    huh = huh.map([](const i32 a) { return a * 2; });
    REQUIRE(huh.get_unchecked() == 20);

    std::ignore = huh.map([](const i32 a) { return a * 2; });
    REQUIRE_THROWS(huh.get_unchecked());

    huh = Error{};
    huh = huh.map([](const i32 a) { return a * 2; });
    REQUIRE(huh.is_err());

    std::ignore = huh.take_err_unchecked();
    REQUIRE_THROWS(std::ignore = huh.map([](const i32 a) { return a * 2; }));
  }

  SECTION("fold") {
    {
      bool first = false, second = false;
      Result<std::tuple<i32, i32>, Error> a = crab::fallible<Error>(
        [&] {
          first = true;
          return 10;
        },
        [&] {
          second = true;
          return 22;
        }
      );
      REQUIRE((first and second));
      REQUIRE(a.is_ok());

      auto [num1, num2] = a.take_unchecked();

      REQUIRE(num1 == 10);
      REQUIRE(num2 == 22);
    }

    Result<std::tuple<i32, i32, i32>, Error> a = crab::fallible<Error>(
      [] -> i32 { return 0; },
      [] -> Result<i32, Error> {
        return Error{};
      },
      [] { return 0; }
    );
    REQUIRE(a.get_err_unchecked() == Error{});
  }
}

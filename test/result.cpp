//
// Created by bishan_ on 4/23/24.
//

#include <catch2/catch_test_macros.hpp>

#include <result.hpp>
#include <pattern_match.hpp>

#include "ref.hpp"

class Error final : public crab::Error {
public:
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
}

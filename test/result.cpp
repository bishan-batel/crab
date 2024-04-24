//
// Created by bishan_ on 4/23/24.
//

#include "result.hpp"

#include <catch2/catch_test_macros.hpp>

class Error final : public crab::Error {
public:
  StringView what() override {
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
}

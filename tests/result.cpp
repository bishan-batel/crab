//
// Created by bishan_ on 4/23/24.
//

#include <catch2/catch_test_macros.hpp>

#include "crab/preamble.hpp"
#include "crab/result/unwrap.hpp"
#include "crab/result/fallible.hpp"
#include "crab/result/Result.hpp"

class Error final : public crab::Error {
public:

  [[nodiscard]] auto operator==(const Error&) const -> bool {
    return true;
  }

  [[nodiscard]] String what() const override {
    return "huh";
  }
};

TEST_CASE("Result", "[result]") {
  SECTION("Ok Values") {
    Result<u32, Error> result{10};

    REQUIRE(result.is_ok());
    REQUIRE_FALSE(result.is_err());

    REQUIRE(result.get() == 10);
    REQUIRE(crab::move(result).unwrap() == 10);

    REQUIRE_THROWS(result.get());
    REQUIRE_THROWS(result.get_err());
    REQUIRE_THROWS(crab::move(result).unwrap());
    REQUIRE_THROWS(crab::move(result).unwrap());

    result = Error{};
    REQUIRE(result.is_err());
    REQUIRE_FALSE(result.is_ok());
    REQUIRE_THROWS(result.get());
    REQUIRE_NOTHROW(result.get_err());

    Error err;
    REQUIRE(result.is_valid());
    REQUIRE_NOTHROW(err = crab::move(result).unwrap_err());

    REQUIRE(not result.is_valid());
    REQUIRE_THROWS(crab::move(result).unwrap_err());
    REQUIRE_THROWS(crab::move(result).unwrap());

    result = 42_u32;
    REQUIRE(result.is_valid());

    REQUIRE(Result<unit, Error>{unit{}}.is_ok());
  }

  SECTION("map") {
    Result<i32, Error> huh{10};
    huh = std::move(huh).map([](const i32 a) { return a * 2; });
    REQUIRE(huh.get() == 20);
    REQUIRE(huh.get() == 20);

    std::ignore = std::move(huh).map([](const i32 a) { return a * 2; });
    REQUIRE_THROWS(huh.get());

    huh = Error{};
    std::ignore = huh.copied().map([](const i32 a) { return a * 2; });
    REQUIRE(huh.is_err());

    std::ignore = std::move(huh).unwrap_err();
    REQUIRE_THROWS(std::ignore = std::move(huh).map([](const i32 a) { return a * 2; }));
  }

  SECTION("fold") {
    {
      bool first = false, second = false;

      // runs each in order until an error
      // if one error, shortciruit and the whole thing is err
      // else, you get a tuple of all the values
      Result<std::tuple<i32, i32>, Error> a = crab::fallible<Error>(
        [&]() {
          first = true;
          return 10;
        },
        [&]() {
          second = true;
          return 22;
        }
      );

      // its just v similar to the ? operator in rust & the do notation in
      // Haskell or just you manually doing every operation and doing the err
      // check and returning for non templated use cases - this for convenience
      // for templated use cases this is literally epic
      REQUIRE((first and second));
      REQUIRE(a.is_ok());

      auto [num1, num2] = a.get();

      REQUIRE(num1 == 10);
      REQUIRE(num2 == 22);
    }

    Result<std::tuple<i32, i32, i32>, Error> a = crab::fallible<Error>(
      []() -> i32 { return 0; },
      []() -> Result<i32, Error> { return Error{}; },
      []() -> i32 { return 0; }
    );
    REQUIRE(a.get_err() == Error{});
  }

  SECTION("and_then") {
    Result<f32, Error> transformed =
      Result<i32, Error>{10}.and_then([](const i32) -> Result<f32, Error> { return 10.f; });
    REQUIRE(transformed.is_ok());
    REQUIRE(transformed.get() == 10.f);

    transformed = Result<i32, Error>{20}.and_then([](const i32) -> Result<f32, Error> { return Error{}; });

    REQUIRE(transformed.is_err());
    REQUIRE(transformed.get_err() == Error{});
  }

  SECTION("String as Errors") {
    const auto non_zero = [](u8 num) -> Result<u8, String> {
      if (num != 0) {
        return num;
      }

      return crab::err<String>("Zero.");
    };

    REQUIRE(non_zero(10).is_ok());

    REQUIRE(non_zero(0).get_err() == String{"Zero."});
  }

  SECTION("Result of references") {

    String str{"Hello"};

    {
      Result<String&, const char*> b{str};

      CHECK(b.is_ok());
      CHECK(b.is_ok_and([](const String& str) { return str == "Hello"; }));

      b = "what";
      CHECK(b.is_err());
      CHECK(b.is_err_and([](StringView err) { return err == "what"; }));
    }
  }
}

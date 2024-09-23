//
// Created by bishan_ on 4/23/24.
//

#include <catch2/catch_test_macros.hpp>

#include <result.hpp>

class Error final : public crab::Error {
public:
  [[nodiscard]] auto operator==(const Error &) const -> bool { return true; }

  [[nodiscard]] String what() const override { return "huh"; }
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
          [&]() {
            first = true;
            return 10;
          },
          [&]() {
            second = true;
            return 22;
          });
      REQUIRE((first and second));
      REQUIRE(a.is_ok());

      auto [num1, num2] = a.take_unchecked();

      REQUIRE(num1 == 10);
      REQUIRE(num2 == 22);
    }

    Result<std::tuple<i32, i32, i32>, Error> a = crab::fallible<Error>(
        []() -> i32 { return 0; }, []() -> Result<i32, Error> { return Error{}; }, []() -> i32 { return 0; });
    REQUIRE(a.get_err_unchecked() == Error{});
  }

  SECTION("and_then") {
    auto transformed = Result<i32, Error>{10}.and_then([](const i32) { return Result<f32, Error>{10.f}; });
    REQUIRE(transformed.is_ok());
    REQUIRE(transformed.get_unchecked() == 10.f);

    transformed = Result<i32, Error>{20}.and_then([](const i32) -> Result<f32, Error> { return Error{}; });

    REQUIRE(transformed.is_err());
    REQUIRE(transformed.get_err_unchecked() == Error{});
  }

  SECTION("String as Errors") {
    const auto non_zero = [](u8 num) -> Result<u8, String> {
      if (num != 0) {
        return crab::ok(num);
      }

      return crab::err<String>("Zero.");
    };

    REQUIRE(non_zero(10).is_ok());

    REQUIRE(non_zero(0).take_err_unchecked() == String{"Zero."});
  }
}

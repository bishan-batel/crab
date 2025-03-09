#include "crab/fn.hpp"
#include <catch2/catch_test_macros.hpp>
#include <tuple>
#include <optional>
#include "test_types.hpp"

TEST_CASE("fn") {

  SECTION("constant") {

    // call test on each of these test typestype here
    test_values(
      []<typename T>(T&& x) {
        const auto generator = crab::fn::constant(std::forward<T>(x));

        const T first = generator();

        for (usize i = 0; i < 5; i++) {
          REQUIRE(generator() == first);
        }

        return unit{};
      },
      true,
      static_cast<i64>(32),
      static_cast<i32>(32),
      static_cast<i16>(32),
      static_cast<i8>(32),
      static_cast<u64>(32),
      static_cast<u32>(32),
      static_cast<u16>(32),
      static_cast<u8>(32),
      static_cast<f32>(32),
      static_cast<f64>(32),
      static_cast<String>("Hello World")
    );
  }

  SECTION("identity") {
    test_values(
      []<typename T>(T&& x) {
        REQUIRE(crab::fn::identity(x) == x);

        T original{x};

        // identity should be idempotent
        for (usize i = 0; i < 10; i++) {
          x = crab::fn::identity(std::forward<T>(x));
        }

        REQUIRE(original == x);

        return unit{};
      },
      true,
      static_cast<i64>(27),
      static_cast<i32>(35),
      static_cast<i16>(30),
      static_cast<i8>(34),
      static_cast<u64>(29),
      static_cast<u32>(32),
      static_cast<u16>(35),
      static_cast<u8>(29),
      static_cast<f32>(36),
      static_cast<f64>(29),
      static_cast<String>("Hello World")
    );

    // test with mondaic operations
    SECTION("monadic") {
      Option<Option<i32>> opt{crab::some(10)};

      REQUIRE(opt.flat_map(crab::fn::identity) == crab::some(10));
      REQUIRE(opt.flat_map(crab::fn::identity) == opt.flatten());
      REQUIRE(Option{10}.map(crab::fn::identity) == Option(10));
      REQUIRE(Option{10}.map(crab::fn::identity) != crab::none);
    }
  }
}

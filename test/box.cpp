#include "box.hpp"

#include <preamble.hpp>
#include <catch2/catch_test_macros.hpp>

#include "option.hpp"

TEST_CASE("Preamble", "[unit]") {
  REQUIRE(unit{} == unit{});
}

TEST_CASE("Option", "[option]") {
  Option a = crab::some(52);

  i32 took;
  REQUIRE_NOTHROW(took = a.take_unchecked());

  REQUIRE(took == 52);

  REQUIRE_THROWS(a.take_unchecked());

  a = crab::some(42);
  REQUIRE(crab::unwrap(std::move(a)) == 42);

  REQUIRE_THROWS(a.take_unchecked());
  // REQUIRE(a.unwrap_or(420) == 420);
}

TEST_CASE("Box", "[box]") {
  SECTION("Box Moving") {
    Box<u32> a = crab::make_box<u32>(10);
    REQUIRE_NOTHROW(a.as_ptr() != nullptr);

    const Box<u32> moved = std::move(a);
    REQUIRE_THROWS(a.as_ptr());
    REQUIRE_NOTHROW(moved.as_ptr());

    REQUIRE(*moved == 10);
    REQUIRE(moved == 10);
  }

  SECTION("Const Correctness") {
    const Box<u32> var = crab::make_box<u32>(42);
    const u32 *ptr = var.as_ptr();
    REQUIRE(*var == 42);
    REQUIRE(*ptr == 42);
  }

  SECTION("Releasing Single") {
    Box<u32> single = crab::make_box<u32>(420);
    u32 *raw_ptr = single.as_ptr();
    REQUIRE(raw_ptr != nullptr);
    REQUIRE(*raw_ptr == 420);

    u32 *b;
    REQUIRE_NOTHROW(b = crab::release(std::move(single)));

    REQUIRE(b == raw_ptr);

    REQUIRE(b != nullptr);

    REQUIRE(*b == 420);

    REQUIRE_THROWS(crab::release(std::move(single)));

    // Generics<G>
    delete b;
  }
}

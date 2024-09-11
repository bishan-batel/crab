#include "box.hpp"

#include <catch2/catch_test_macros.hpp>
#include <preamble.hpp>

struct SelfReferential {
private:
  Option<Box<SelfReferential>> test;
};

TEST_CASE("Preamble", "[unit]") {
  REQUIRE(unit{} == unit{});
  REQUIRE(unit::val == unit{});
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

  static_assert(sizeof(Box<u32>) == sizeof(u32 *));
  // static_assert(sizeof(Box<u32[]>) == sizeof(u32 *) + sizeof(std::nullptr_t));

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

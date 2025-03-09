#include <catch2/catch_test_macros.hpp>
#include <preamble.hpp>
#include "ref.hpp"
#include "test_types.hpp"

TEST_CASE("crab::is & crab::is_exact") {
  Derived a;
  Base b;

  REQUIRE(crab::ref::is<Derived>(a));
  REQUIRE(crab::ref::is_exact<Derived>(a));

  REQUIRE(crab::ref::is<Base>(a));
  REQUIRE(crab::ref::is<Base>(b));
  REQUIRE(not crab::ref::is_exact<Base>(a));
}

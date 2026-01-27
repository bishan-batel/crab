#include <catch2/catch_test_macros.hpp>
#include <crab/preamble.hpp>
#include <crab/ref/ref.hpp>
#include <crab/ref/is.hpp>
#include <crab/ref/is_exact.hpp>
#include "test_types.hpp"

TEST_CASE("crab::ref::is") {
  Derived a;
  Base b;

  REQUIRE(crab::ref::is<Derived>(a));
  REQUIRE(crab::ref::is<Base>(a));
  REQUIRE(crab::ref::is<Base>(b));
}

TEST_CASE("crab::ref::is_exact") {
  Derived a;
  Base b;

  REQUIRE(crab::ref::is_exact<Derived>(a));
  REQUIRE(not crab::ref::is_exact<Base>(a));
}

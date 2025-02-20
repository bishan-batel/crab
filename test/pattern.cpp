#include <catch2/catch_test_macros.hpp>
#include <preamble.hpp>
#include "ref.hpp"
#include "test_types.hpp"

TEST_CASE("crab::is & crab::is_exact") {
  ex::Derived a;
  ex::Base b;

  REQUIRE(crab::ref::is<ex::Derived>(a));
  REQUIRE(crab::ref::is_exact<ex::Derived>(a));

  REQUIRE(crab::ref::is<ex::Base>(a));
  REQUIRE(crab::ref::is<ex::Base>(b));
  REQUIRE(not crab::ref::is_exact<ex::Base>(a));
}

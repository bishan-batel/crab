//
// Created by bishan_ on 5/20/24.
//

#include "rc.hpp"

#include <catch2/catch_test_macros.hpp>
#include "test_types.hpp"

#if CATCH2_TESTS
int a
#endif

TEST_CASE("Rc/RcMut") {
  SECTION("move") {
    SECTION("Rc") {
      Rc<String> rc = crab::make_rc<String>("some str");

      REQUIRE(rc.get_ref_count() == 1);
      REQUIRE(rc.is_unique());

      REQUIRE_NOTHROW(*rc == "some str");

      Rc<String> other = std::move(rc);
      REQUIRE(other.get_ref_count() == 1);
      REQUIRE(other.is_unique());

      REQUIRE(rc.get_ref_count() == 0);
      REQUIRE(not rc.is_valid());
    }

    SECTION("RcMut") {
      RcMut<String> rc = crab::make_rc_mut<String>("some str");

      REQUIRE(rc.get_ref_count() == 1);
      REQUIRE(rc.is_unique());

      REQUIRE_NOTHROW(*rc == "some str");

      RcMut<String> other = std::move(rc);
      REQUIRE(other.get_ref_count() == 1);
      REQUIRE(other.is_unique());

      REQUIRE(rc.get_ref_count() == 0);
      REQUIRE(not rc.is_valid());
    }
  }

  SECTION("Downcast") {
    SECTION("Rc") {
      Rc<ex::Base> original = crab::make_rc<ex::Derived>();
      Option<Rc<ex::Derived>> returned = original.downcast<ex::Derived>();

      REQUIRE(not original.is_unique());
      REQUIRE(original.get_ref_count() == 2);
      REQUIRE_NOTHROW(std::move(returned).unwrap());
      REQUIRE(original.is_unique());
    }

    SECTION("RcMut") {
      RcMut<ex::Base> original = crab::make_rc_mut<ex::Derived>();
      Option<RcMut<ex::Derived>> returned = original.downcast<ex::Derived>();

      REQUIRE(not original.is_unique());
      REQUIRE(original.get_ref_count() == 2);
      REQUIRE_NOTHROW(std::move(returned).unwrap());
      REQUIRE(original.is_unique());
    }
  }
}

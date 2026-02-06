//
// Created by bishan_ on 5/20/24.
//

#include <catch2/catch_test_macros.hpp>
#include <utility>
#include "test_types.hpp"

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

      REQUIRE(not rc.is_valid());
    }
  }

  SECTION("Downcast") {
    SECTION("Rc") {
      Rc<Base> original = crab::make_rc<Derived>();
      Option<Rc<Derived>> returned = original.downcast<Derived>();

      REQUIRE(not original.is_unique());
      REQUIRE(original.get_ref_count() == 2);
      REQUIRE_NOTHROW(std::move(returned).unwrap());
      REQUIRE(original.is_unique());

      returned = original.downcast<Derived>();

      Rc<Derived> moved{std::move(returned.get())};
      REQUIRE(returned.is_none());
      REQUIRE(moved.is_valid());
    }

    SECTION("RcMut") {
      RcMut<Base> original = crab::make_rc_mut<Derived>();
      Option<RcMut<Derived>> returned = original.downcast<Derived>();

      REQUIRE(not original.is_unique());
      REQUIRE(original.get_ref_count() == 2);
      REQUIRE_NOTHROW(std::move(returned).unwrap());
      REQUIRE(original.is_unique());
    }
  }

  SECTION("Option Niche Optimisation") {
    using crab::mem::size_of;

    asserts::for_types(asserts::common_types, []<typename T>(asserts::type<T>) {
      STATIC_REQUIRE(sizeof(Rc<T>) == sizeof(RcMut<T>));
      STATIC_REQUIRE(sizeof(Rc<T>) == sizeof(Option<Rc<T>>));
      STATIC_REQUIRE(sizeof(RcMut<T>) == sizeof(Option<RcMut<T>>));
    });
  }
}

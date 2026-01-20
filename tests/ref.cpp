#include <crab/ref/ref.hpp>
#include <crab/type_traits.hpp>
#include <catch2/catch_test_macros.hpp>
#include "test_static_asserts.hpp"
#include "test_types.hpp"

struct IncompleteType;

TEST_CASE("Type Traits") {

  STATIC_REQUIRE(not crab::complete_type<IncompleteType>);


  asserts::for_types(asserts::common_types, []<typename T>(asserts::type<T>) {
    STATIC_REQUIRE(crab::complete_type<String>);
  });
}

namespace ty = crab::ty;

TEST_CASE("Ref Types") {
  SECTION("prvalue-limited constructrion") {

    SECTION("Ref") {

      i32 a{10};
      const i32 b{10};

      STATIC_REQUIRE(ty::same_as<Ref<i32>, decltype(Ref(a))>);
      STATIC_REQUIRE(ty::same_as<Ref<i32>, decltype(Ref(b))>);

      // Construction from a prvalue must not be valid.
      STATIC_REQUIRE(not ty::convertible<decltype(10), Ref<i32>>);
      STATIC_REQUIRE(not ty::convertible<i32, Ref<i32>>);

      STATIC_REQUIRE(ty::convertible<i32&, Ref<i32>>);
      STATIC_REQUIRE(ty::convertible<const i32&, Ref<i32>>);
    }

    SECTION("RefMut") {
      i32 a{10};

      STATIC_REQUIRE(ty::same_as<RefMut<i32>, decltype(RefMut(a))>);

      // Construction from a prvalue must not be valid.
      STATIC_REQUIRE(not ty::convertible<decltype(10), RefMut<i32>>);
      STATIC_REQUIRE(not ty::convertible<i32, RefMut<i32>>);

      STATIC_REQUIRE(ty::convertible<i32&, RefMut<i32>>);
    }
  }
}

struct IncompleteType {};

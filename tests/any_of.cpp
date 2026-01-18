
#include <alloca.h>
#include <catch2/catch_test_macros.hpp>

#include "crab/core/discard.hpp"
#include "crab/preamble.hpp"
#include "crab/any/AnyOf.hpp"
#include "crab/type_traits.hpp"
#include "test_static_asserts.hpp"
#include "test_types.hpp"

using crab::any::AnyOf;

TEST_CASE("AnyOf<Ts...>:IndexOf", "[anyof]") {
  using T = AnyOf<i32, u32, f32, MoveOnly>;

  STATIC_CHECK(T::IndexOf<i32> == 0);
  STATIC_CHECK(T::IndexOf<u32> == 1);
  STATIC_CHECK(T::IndexOf<f32> == 2);
  STATIC_CHECK(T::IndexOf<MoveOnly> == 3);

  {
    T value{i32{}};
    CHECK(T::IndexOf<i32> == value.get_index());
  }

  {
    T value{u32{}};
    CHECK(T::IndexOf<u32> == value.get_index());
  }

  {
    T value{f32{}};
    CHECK(T::IndexOf<f32> == value.get_index());
  }

  {
    T value{MoveOnly{"hello"}};
    CHECK(T::IndexOf<MoveOnly> == value.get_index());
  }
}

TEST_CASE("AnyOf<Ts...>:NthType", "[anyof]") {
  namespace ty = crab::ty;

  using T = AnyOf<i32, u32, f32, MoveOnly>;

  STATIC_CHECK(ty::same_as<T::NthType<T::IndexOf<i32>>, i32>);
  STATIC_CHECK(ty::same_as<T::NthType<T::IndexOf<u32>>, u32>);
  STATIC_CHECK(ty::same_as<T::NthType<T::IndexOf<f32>>, f32>);
  STATIC_CHECK(ty::same_as<T::NthType<T::IndexOf<MoveOnly>>, MoveOnly>);
}

TEST_CASE("AnyOf") {
  using T = AnyOf<i32, u32, f32, MoveOnly>;
  T value{MoveOnly{"hello"}};

  CHECK(value.as<MoveOnly>().is_some_and([](const MoveOnly& o) { return o.get_name() == "hello"; }));

  SECTION("match / visit") {
    using AnyOf = crab::any::AnyOf<i32, u32, f32, MoveOnly>;

    SECTION("mut") {
      AnyOf value{MoveOnly{"hello"}};

      value.match([](MoveOnly& m) { m.set_name("world"); }, [](auto&&) {});

      REQUIRE(value.is<MoveOnly>());
      CHECK(value.as<MoveOnly>().unwrap().get_name() == "world");
    }

    SECTION("const") {
      const AnyOf value{MoveOnly{"hello"}};

      CHECK(
        value.match([](const MoveOnly& bruh) { return bruh.get_name(); }, [](const auto&) { return "bruh"; }) == "hello"
      );
    }

    SECTION("visit returning reference type") {
      const AnyOf value{MoveOnly{"hello"}};

      String outer{"world"};

      const crab::cases visitor{
        [outer](const MoveOnly&) -> const String& { return outer; },
        [outer](const auto&) -> const String& { return outer; },
      };

      const String& ref = value.visit<const decltype(visitor)&, const String&>(visitor);
      CHECK(&ref == &outer);
    }
  }

  /*
  SECTION("Reference types") {
    const u32 a{10};
    u32 b{10};

    {
      AnyOf<const u32&> value{a};
      CHECK(value.is<const u32&>());
    }

    {
      AnyOf<const u32&> value{b};
      CHECK(value.is<const u32&>());
    }

    {
      AnyOf<const u32&, u32&> value{a};
      CHECK(value.is<const u32&>());
    }

    {
      AnyOf<const u32&, u32&> value{b};
      CHECK(value.is<u32&>());
    }

    {
      AnyOf<const u32&, u32&> value{crab::implicit_cast<const u32&>(b)};
      CHECK(value.is<const u32&>());
    }

    {
      auto value{
        AnyOf<const u32&, u32&>::from<const u32&>(b),
      };
      CHECK(value.is<const u32&>());
    }
  }
  */
}

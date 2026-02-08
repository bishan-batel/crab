
#include <catch2/catch_test_macros.hpp>
#include <type_traits>

#include "crab/core/discard.hpp"
#include "crab/preamble.hpp"
#include "crab/any/AnyOf.hpp"
#include "test_static_asserts.hpp"
#include "test_types.hpp"

using crab::any::AnyOf;

TEST_CASE("AnyOf::NumTypes", "[anyof]") {
  // unique type helper
#define T std::integral_constant<usize, __COUNTER__>

  // simple brute force check, i dont care enough for cases past 24 lol

  STATIC_CHECK(AnyOf<T>::NumTypes == 1);
  STATIC_CHECK(AnyOf<T, T>::NumTypes == 2);
  STATIC_CHECK(AnyOf<T, T, T>::NumTypes == 3);
  STATIC_CHECK(AnyOf<T, T, T, T>::NumTypes == 4);
  STATIC_CHECK(AnyOf<T, T, T, T, T>::NumTypes == 5);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T>::NumTypes == 6);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T>::NumTypes == 7);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T>::NumTypes == 8);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T>::NumTypes == 9);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T>::NumTypes == 10);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 11);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 12);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 13);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 14);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 15);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 16);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 17);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 18);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 19);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 20);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 21);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 22);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 23);
  STATIC_CHECK(AnyOf<T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T>::NumTypes == 24);

#undef T
}

TEST_CASE("AnyOf<Ts...>::DataSize", "[anyof]") {
  STATIC_CHECK(AnyOf<u32>::DataSize == sizeof(i32));
  STATIC_CHECK(AnyOf<i64>::DataSize == sizeof(i64));
  STATIC_CHECK(AnyOf<u32, i64>::DataSize == sizeof(i64));
  STATIC_CHECK(AnyOf<u8, i64>::DataSize == sizeof(i64));
  STATIC_CHECK(AnyOf<std::array<u32, 24>, u32>::DataSize == sizeof(u32) * 24);
}

TEST_CASE("AnyOf<Ts...>::Alignment", "[anyof]") {
  STATIC_CHECK(AnyOf<u32>::Alignment == alignof(i32));
  STATIC_CHECK(AnyOf<i64>::Alignment == alignof(i64));
  STATIC_CHECK(AnyOf<u32, i64>::Alignment == alignof(i64));
  STATIC_CHECK(AnyOf<u8, i64>::Alignment == alignof(i64));
  STATIC_CHECK(AnyOf<std::array<u32, 24>, u32>::Alignment == alignof(u32));
}

TEST_CASE("AnyOf<Ts...>:IndexOf", "[anyof]") {
  using T = AnyOf<i32, u32, f32, MoveOnly>;

  STATIC_CHECK(T::IndexOf<i32> == 0);
  STATIC_CHECK(T::IndexOf<u32> == 1);
  STATIC_CHECK(T::IndexOf<f32> == 2);
  STATIC_CHECK(T::IndexOf<MoveOnly> == 3);

  SECTION("get_index()") {
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
}

TEST_CASE("AnyOf as", "[anyof]") {
  using T = AnyOf<i32, u32, f32, MoveOnly>;
  T value{MoveOnly{"hello"}};

  CHECK(value.as<MoveOnly>().is_some_and([](const MoveOnly& o) { return o.get_name() == "hello"; }));
}

TEST_CASE("AnyOf<Ts...> match/visit", "[anyof]") {
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
      [&](const MoveOnly&) -> const String& { return outer; },
      [&](const auto&) -> const String& { return outer; },
    };

    const String& ref = value.visit<const decltype(visitor)&, const String&>(visitor);
    CHECK(&ref == &outer);
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

TEST_CASE("Reference Types") {

  SECTION("construction & as") {
    const u32 a{10};
    u32 b{10};

    {
      AnyOf<const u32&> value{a};
      CHECK(value.is<const u32&>());
      CHECK(&value.as<const u32&>().unwrap() == &a);
    }

    {
      AnyOf<const u32&> value{b};
      CHECK(value.is<const u32&>());
      CHECK(&value.as<const u32&>().unwrap() == &b);
    }

    {
      AnyOf<const u32&, u32&> value{a};
      CHECK(value.is<const u32&>());
      CHECK(not value.is<u32&>());
      CHECK(&value.as<const u32&>().unwrap() == &a);
    }

    {
      AnyOf<const u32&, u32&> value{b};
      CHECK(value.is<u32&>());
      CHECK(not value.is<const u32&>());
      CHECK(&value.as<u32&>().unwrap() == &b);
    }

    {
      AnyOf<const u32&, u32&> value{crab::implicit_cast<const u32&>(b)};
      CHECK(value.is<const u32&>());
      CHECK(not value.is<u32&>());
    }

    {
      auto value{
        AnyOf<const u32&, u32&>::from<const u32&>(b),
      };
      CHECK(value.is<const u32&>());
    }
  }

  SECTION("operator=") {
    const u32 a{10};
    u32 b{10};

    AnyOf<const u32&, u32&> value{a};
    CHECK(&value.as<const u32&>().unwrap() == &a);

    value = a;
    CHECK(&value.as<const u32&>().unwrap() == &a);

    value = b;

    CHECK(&value.as<u32&>().unwrap() == &b);

    value = crab::implicit_cast<const u32&>(b);

    CHECK(&value.as<const u32&>().unwrap() == &b);
  }

  SECTION("insert") {
    const u32 a{10};
    u32 b{10};

    AnyOf<const u32&, u32&> value{a};
    CHECK(&value.as<const u32&>().unwrap() == &a);

    value.insert(a);
    CHECK(&value.as<const u32&>().unwrap() == &a);

    value.insert(b);

    CHECK(&value.as<u32&>().unwrap() == &b);

    value.insert<const u32&>(b);

    CHECK(&value.as<const u32&>().unwrap() == &b);
  }

  SECTION("emplace") {
    const u32 a{10};
    u32 b{10};

    {
      AnyOf<const u32&, u32&> value{a};
      CHECK(&value.as<const u32&>().unwrap() == &a);

      value.emplace<const u32&>(a);
      CHECK(&value.as<const u32&>().unwrap() == &a);

      value.emplace<u32&>(b);

      CHECK(&value.as<u32&>().unwrap() == &b);
    }
  }
}

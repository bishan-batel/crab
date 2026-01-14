
#include <catch2/catch_test_macros.hpp>

#include "crab/core/discard.hpp"
#include "crab/preamble.hpp"
#include "crab/any/AnyOf.hpp"
#include "test_static_asserts.hpp"
#include "test_types.hpp"

using crab::any::AnyOf;

TEST_CASE("AnyOf") {

  SECTION("IndexOf") {
    using T = AnyOf<i32, u32, f32, MoveOnly>;

    STATIC_CHECK(T::IndexOf<i32> == 0);
    STATIC_CHECK(T::IndexOf<u32> == 1);
    STATIC_CHECK(T::IndexOf<f32> == 2);
    STATIC_CHECK(T::IndexOf<MoveOnly> == 3);

    {
      T value{i32{10}};
      CHECK(T::IndexOf<i32> == value.get_index());
    }
    {
      T value{u32{10}};
      CHECK(T::IndexOf<u32> == value.get_index());
    }
    {
      T value{f32{10}};
      CHECK(T::IndexOf<f32> == value.get_index());
    }
    {
      T value{MoveOnly{"hello"}};
      CHECK(T::IndexOf<MoveOnly> == value.get_index());
    }

    T value{MoveOnly{"hello"}};

    CHECK(value.as<MoveOnly>().is_some_and([](const MoveOnly& o) { return o.get_name() == "hello"; }));
  }
  SECTION("match") {
    using AnyOf = crab::any::AnyOf<i32, u32, f32, MoveOnly>;

    AnyOf value{MoveOnly{"hello"}};
    u8 tag{0};

    String str = value.match([](const MoveOnly& bruh) { return bruh.get_name(); }, [](const auto&) { return "bruh"; });

    crab::discard(tag, str);

    CHECK(str == "hello");
  }
}

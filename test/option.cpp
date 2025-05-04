#include <algorithm>
#include <result.hpp>
#include <option.hpp>
#include <catch2/catch_test_macros.hpp>
#include <functional>
#include "box.hpp"
#include "test_static_asserts.hpp"
#include "test_types.hpp"
#include <preamble.hpp>
#include <ref.hpp>
#include <crab/fn.hpp>
#include <range.hpp>

TEST_CASE("Option", "Tests for all option methods") {
  const auto life = crab::fn::constant(42);

  STATIC_CHECK(sizeof(crab::None) == 1);

  SECTION("Constructors & Move Semantics") {

    // general construction
    assert::for_types(assert::common_types, []<typename T>(assert::type<T>) {
      constexpr bool copyable = std::copyable<T>;

      using Option = ::Option<MoveTracker<T>>;

      SECTION("Construction from T&& and None") {
        RcMut<MoveCount> counter = crab::make_rc_mut<MoveCount>();
        MoveCount expected{0, 0};

        MoveTracker<T> value{MoveTracker<T>::from(counter)};

        CHECK(Option{crab::none}.is_none());
        CHECK_THROWS(Option{crab::none}.unwrap());

        counter->valid(expected);

        CHECK(Option{std::move(value)}.is_some());
        CHECK_NOTHROW(Option{std::move(value)}.unwrap());

        if constexpr (copyable) {
          CHECK(Option{value}.is_some());
          CHECK_NOTHROW(Option{value}.unwrap());
        }
      }

      SECTION("Move limits") {
        RcMut<MoveCount> counter = crab::make_rc_mut<MoveCount>();
        MoveTracker<T> value{MoveTracker<T>::from(counter)};
        MoveCount expected{0, 0};

        expected.moves++;
        Option opt{std::move(value)};

        counter->valid(expected);

        // explicit constructor, std::move, and assignmnet

        expected.moves += 3;
        CHECK_NOTHROW(opt = MoveTracker<T>(std::move(opt).unwrap()));
        //
        if constexpr (copyable) {
          // rvalue from made copy, then assignment
          expected.moves++;
          expected.copies++;
          CHECK_NOTHROW(opt = Option(opt));

          counter->valid(expected);
        }
      }
    });
  }

  SECTION("Bool conversion") {
    Option<i32> opt{0};

    CHECK(opt.is_some());
    CHECK(static_cast<bool>(opt));
    CHECK(static_cast<const bool&>(opt));
    CHECK(opt);

    opt = crab::none;
    CHECK_FALSE(opt.is_some());
    CHECK_FALSE(static_cast<bool>(opt));
    CHECK_FALSE(opt);
  }

  SECTION("take_or variants") {
    CHECK(Option{10}.take_or_default() == 10);
    CHECK(Option<i32>{}.take_or_default() == 0);

    CHECK(Option{MoveOnly{"moment"}}.take_or_default().get_name() == "moment");
    CHECK(Option<MoveOnly>{}.take_or_default().get_name() == "");

    CHECK(Option{10}.take_or(42) == 10);
    CHECK(Option<i32>{}.take_or(42) == 42);

    CHECK(Option{10}.take_or(life) == 10);
    CHECK(Option<i32>{}.take_or(life) == life());

    const auto gen = []() { return MoveOnly{"default"}; };

    CHECK(Option{MoveOnly{"name"}}.take_or(gen()).get_name() == "name");
    CHECK(Option<MoveOnly>{}.take_or(gen()).get_name() == "default");

    CHECK(Option{MoveOnly{"name"}}.take_or(gen).get_name() == "name");
    CHECK(Option<MoveOnly>{}.take_or(gen).get_name() == "default");
  }

  SECTION("get_or variants") {
    CHECK(Option{10}.get_or(42) == 10);
    CHECK(Option<i32>{}.get_or(42) == 42);

    CHECK(Option{10}.get_or_default() == 10);
    CHECK(Option<i32>{}.get_or_default() == 0);

    CHECK(Option<String>{"what"}.get_or_default() == "what");
    CHECK(Option<String>{}.get_or_default() == "");
  }

  SECTION("as_ref and as_mut") {
    Option<i32> value{10};
    CHECK(value.is_some());

    CHECK_NOTHROW(value.as_ref().unwrap() == 10);
    CHECK_NOTHROW(value.as_mut().unwrap() = 42);
    CHECK(value == Option{42});
  }

  SECTION("Functor methods between Option and Result") {
    CHECK(Option{10}.ok_or<const char*>("what").unwrap() == 10);
    CHECK(Option<i32>{}.ok_or<String>("what").unwrap_err() == "what");

    CHECK(Option{10}.take_ok_or<String>("what").unwrap() == 10);
    CHECK(Option<i32>{}.take_ok_or<String>("what").unwrap_err() == "what");
  }

  SECTION("filter") {
    CHECK(Option{10}.filter(crab::fn::is_even).is_some());
    CHECK(Option<i32>{}.filter(crab::fn::is_even).is_none());

    CHECK(Option{10}.filter(crab::fn::is_odd).is_none());
    CHECK(Option<i32>{}.filter(crab::fn::is_odd).is_none());

    CHECK(Option<String>{""}.filter(&String::empty).is_some());
    CHECK(Option<String>{}.filter(&String::empty).is_none());
    CHECK(Option<String>{""}.filter(&String::empty).is_some());
    CHECK(Option<String>{"moment"}.map(&String::size) == Option<usize>(6));
    CHECK(Option<String>{}.map(&String::size) == Option<usize>());
  }

  SECTION("map") {
    CHECK(Option{10}.map(crab::fn::constant(42)) == Option{42});
    CHECK(Option<i32>{}.map(crab::fn::constant(42)) == crab::none);

    CHECK(Option<RefMut<Derived>>{}.map<Base>() == crab::none);
    CHECK(Option<RefMut<Derived>>{}.map<Derived>() == crab::none);
    CHECK(Option<RefMut<Derived>>{}.map<Derived>() == crab::none);
    CHECK(
      Option<RefMut<Base>>().flat_map(crab::fn::cast<Derived>) == crab::none
    );
  }

  SECTION("as_ref/as_mut") {
    SECTION("copyable") {
      Option<String> name{"Hello"};
      Option<RefMut<String>> ref = name.as_mut();
      Option<Ref<String>> ref_mut = name.as_ref();

      REQUIRE(name.is_some());
      REQUIRE(ref.is_some());
      REQUIRE(ref_mut.is_some());

      REQUIRE(*ref.get_unchecked() == "Hello");
      REQUIRE(*ref_mut.get_unchecked() == "Hello");

      name.get_unchecked() += " World";

      REQUIRE(*ref.get_unchecked() == "Hello World");
      REQUIRE(*ref_mut.get_unchecked() == "Hello World");
    }

    SECTION("move only") {
      Option<MoveOnly> name{MoveOnly{"Hello"}};
      Option<RefMut<MoveOnly>> ref = name.as_mut();
      Option<Ref<MoveOnly>> ref_mut = name.as_ref();

      REQUIRE(name.is_some());
      REQUIRE(ref.is_some());
      REQUIRE(ref_mut.is_some());

      REQUIRE(ref.get_unchecked()->get_name() == "Hello");
      REQUIRE(ref_mut.get_unchecked()->get_name() == "Hello");

      name.get_unchecked().set_name("Hello World");

      REQUIRE(ref.get_unchecked()->get_name() == "Hello World");
      REQUIRE(ref_mut.get_unchecked()->get_name() == "Hello World");
    }

    SECTION("reference types") {

      assert::for_types<i32&, const i32&>([]<typename T>(assert::type<T>) {
        Option<T> a;

        REQUIRE(a.is_none());
        REQUIRE_THROWS(a.get_unchecked());

        i32 i = 10;
        i32 j = 10;

        a = i;

        REQUIRE(a.is_some());
        CHECK(a.get_unchecked() == i);
        CHECK(a.get_unchecked() == 10);

        CHECK(a != crab::none);
        CHECK(a == Option<T>{a});
        CHECK(a == Option<T>{i});
        CHECK(a == Option<T>{j});

        REQUIRE_NOTHROW(a.template map<i32>().unwrap() == 10);
      });

      Option<i32&> a;

      REQUIRE(a.is_none());
      REQUIRE_THROWS(a.get_unchecked());

      i32 i = 10;
      i32 j = 10;

      a = i;

      a.get_unchecked() = 11;

      REQUIRE(a.is_some());
      CHECK(a.get_unchecked() == i);
      CHECK(a.get_unchecked() == 11);

      CHECK(a != crab::none);
      CHECK(a == Option<i32&>{a});
      CHECK(a == Option<i32&>{i});
      CHECK(a != Option<i32&>{j});

      REQUIRE_NOTHROW(a.filter(crab::fn::constant(true)));

      REQUIRE_NOTHROW(a.template map<i32>().unwrap() == 11);
    }
  }
}

#include <algorithm>
#include <concepts>
#include <catch2/catch_test_macros.hpp>
#include "crab/fn/cast.hpp"
#include "crab/result/Result.hpp"
#include "crab/fn/identity.hpp"
#include "test_static_asserts.hpp"
#include "test_types.hpp"
#include <crab/boxed/Box.hpp>
#include <crab/ref/ref.hpp>
#include <crab/num/range.hpp>
#include <crab/opt/Option.hpp>
#include <crab/opt/some.hpp>
#include <utility>

namespace ty = crab::ty;

TEST_CASE("Option", "Tests for all option methods") {
  const auto life = crab::fn::constant(42);

  STATIC_CHECK(sizeof(crab::opt::None) == 1);

  SECTION("Option<T>::GenericStorage Reference Optimisation") {
    asserts::for_types(asserts::common_types, []<typename T>(asserts::type<T>) {
      STATIC_CHECK(sizeof(T*) == sizeof(Option<T&>));
      STATIC_CHECK(sizeof(T*) == sizeof(Option<const T&>));
    });
  }

  SECTION("Constructors & Move Semantics") {
    Option<i32> a, b;

    auto c = a and b;

    // general construction
    asserts::for_types(asserts::common_types, []<typename T>(asserts::type<T>) {
      constexpr bool copyable{ty::copyable<T>};

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

        expected.moves += 2;
        CHECK_NOTHROW(opt = MoveTracker<T>(std::move(opt).unwrap()));

        counter->valid(expected);

        //
        if constexpr (copyable) {
          // rvalue from made copy, then assignment
          expected.moves++;
          expected.copies++;
          CHECK_NOTHROW(opt = Option(opt));

          counter->valid(expected);

          CHECK_NOTHROW(opt = opt);
          CHECK(opt.is_some());

          counter->valid(expected);

          Option copy{opt};
          expected.copies++;
          counter->valid(expected);

          CHECK_NOTHROW(opt = copy);
          expected.copies++;

          counter->valid(expected);
          CHECK(opt.is_some());
        }
      }
    });
  }

  SECTION("Bool conversion") {
    Option<i32> opt{0};

    CHECK(opt.is_some());
    CHECK(static_cast<bool>(opt));
    CHECK(opt);

    opt = crab::none;
    CHECK_FALSE(opt.is_some());
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
    CHECK(Option<String>{""}.filter(&String::empty).is_some());
    CHECK(Option<String>{}.filter(&String::empty).is_none());
    CHECK(Option<String>{""}.filter(&String::empty).is_some());
    CHECK(Option<String>{"moment"}.map(&String::size) == Option<usize>(6));
    CHECK(Option<String>{}.map(&String::size) == Option<usize>());
  }

  SECTION("map") {
    CHECK(Option{10}.map(crab::fn::constant(42)) == Option{42});
    CHECK(Option<i32>{}.map(crab::fn::constant(42)) == crab::none);

    CHECK(Option<Derived&>{}.map<Base>() == crab::none);
    CHECK(Option<Derived&>{}.map<Derived>() == crab::none);
    CHECK(Option<Derived&>{}.map<Derived>() == crab::none);

    auto c = crab::fn::cast<Derived>;
    CHECK(Option<Derived&>{}.flat_map(c) == crab::none);
  }

  SECTION("as_ref/as_mut") {
    SECTION("copyable") {
      Option<String> name{"Hello"};
      Option<String&> ref = name.as_mut();
      Option<const String&> ref_mut = name.as_ref();

      REQUIRE(name.is_some());
      REQUIRE(ref.is_some());
      REQUIRE(ref_mut.is_some());

      REQUIRE(ref.get() == "Hello");
      REQUIRE(ref_mut.get() == "Hello");

      name.get() += " World";

      REQUIRE(ref.get() == "Hello World");
      REQUIRE(ref_mut.get() == "Hello World");
    }

    SECTION("move only") {
      Option<MoveOnly> name{MoveOnly{"Hello"}};
      Option<MoveOnly&> ref = name.as_mut();
      Option<const MoveOnly&> ref_mut = name.as_ref();

      REQUIRE(name.is_some());
      REQUIRE(ref.is_some());
      REQUIRE(ref_mut.is_some());

      REQUIRE(ref.get().get_name() == "Hello");
      REQUIRE(ref_mut.get().get_name() == "Hello");

      name.get().set_name("Hello World");

      REQUIRE(ref.get().get_name() == "Hello World");
      REQUIRE(ref_mut.get().get_name() == "Hello World");
    }
  }

  SECTION("flatten") {
    asserts::for_types(asserts::common_types, []<typename T>(asserts::type<T>) {
      STATIC_REQUIRE(not requires(crab::opt::Option<T> opt) { opt.flatten(); });
      STATIC_REQUIRE(requires(Option<Option<T>> opt) { Option<T>{std::move(opt).flatten()}; });
    });
  }

  SECTION("crab::some implicit vs explicit template") {
    asserts::for_types(asserts::common_types, []<typename T>(asserts::type<T>) {
      asserts::for_types(asserts::types<T, T&, const T&>, []<typename K>(asserts::type<K>) {
        if constexpr (std::copy_constructible<T>) {
          STATIC_REQUIRE(ty::same_as<Option<T>, decltype(crab::some(std::declval<K>()))>);
        }

        STATIC_REQUIRE(ty::same_as<Option<K>, decltype(crab::some<K>(std::declval<K>()))>);
      });
    });
  }
}

TEST_CASE("Reference Types", "[option]") {

  asserts::for_types<i32&, const i32&>([]<typename T>(asserts::type<T>) {
    Option<T> a;

    CHECK(a.is_none());

    i32 i = 10;
    i32 j = 10;

    a = i;

    CHECK(a.is_some());
    CHECK(a.get() == i);
    CHECK(a.get() == 10);

    CHECK(a != crab::none);
    CHECK(a == Option<T>{a});
    CHECK(a == Option<T>{i});
    CHECK(a == Option<T>{j});

    CHECK(a.template map<i32>().unwrap() == 10);
  });

  Option<i32&> a;

  REQUIRE(a.is_none());
  CHECK_THROWS(a.get());

  i32 i = 10;
  i32 j = 10;

  a = i;

  a.get() = 11;

  REQUIRE(a.is_some());
  CHECK(a.get() == i);
  CHECK(a.get() == 11);

  CHECK(a != crab::none);
  CHECK(a == Option<i32&>{a});
  CHECK(a == Option<i32&>{i});
  CHECK(a != Option<i32&>{j});

  REQUIRE_NOTHROW(a.filter(crab::fn::constant(true)));

  REQUIRE_NOTHROW(a.template map<i32>().unwrap() == 11);
}

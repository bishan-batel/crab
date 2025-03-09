#include <algorithm>
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

TEST_CASE("Option", "[option]") {
  STATIC_CHECK(sizeof(crab::None) == 1);

  SECTION("Type traits") {
    // is ref helpers
    assert::for_types(assert::common_types, []<typename T>(assert::type<T>) {
      namespace ref = crab::ref;

      // none of these types should be triggered as ref / ref mut
      assert::for_types(
        assert::types<const T&, T&, T*, const T*, T>,
        []<typename K>(assert::type<K>) {
          STATIC_CHECK(not ref::is_ref_type<K>::value);
          STATIC_CHECK(not ref::is_ref_mut_type<K>::value);
        }
      );

      assert::for_types(assert::ref_types<T>, []<typename K>(assert::type<K>) {
        // normal types should not trigger these flags

        using underlying = typename ref::decay_type<K>::underlying_type;

        STATIC_CHECK(std::same_as<underlying, T>);
      });

      STATIC_CHECK(ref::is_ref_type<Ref<T>>::value);
      STATIC_CHECK(not ref::is_ref_mut_type<Ref<T>>::value);

      STATIC_CHECK(not ref::is_ref_type<RefMut<T>>::value);
      STATIC_CHECK(ref::is_ref_mut_type<RefMut<T>>::value);
    });
  }

  SECTION("Constructors & Move Semantics") {

    // general construction
    assert::for_types(assert::common_types, []<typename T>(assert::type<T>) {
      constexpr bool copyable = std::copyable<T>;

      using Option = ::Option<MoveTracker<T>>;

      SECTION("Construction from T&& and None") {
        RcMut<MoveCount> counter = crab::make_rc_mut<MoveCount>();

        MoveTracker<T> value{MoveTracker<T>::from(counter)};

        REQUIRE(Option{crab::none}.is_none());
        REQUIRE_THROWS(Option{crab::none}.unwrap());

        REQUIRE(counter->copies == 0);
        REQUIRE(counter->moves == 0);

        REQUIRE(Option{std::move(value)}.is_some());
        REQUIRE_NOTHROW(Option{std::move(value)}.unwrap());
      }

      SECTION("Move limits") {
        RcMut<MoveCount> counter = crab::make_rc_mut<MoveCount>();
        MoveTracker<T> value{MoveTracker<T>::from(counter)};
        MoveCount expected{0, 0};

        expected.moves++;
        Option opt{std::move(value)};

        counter->valid(expected);

        expected.moves++;
        opt = std::move(opt);

        counter->valid(expected);

        if constexpr (copyable) {
          expected.moves += 2;
          expected.copies++;
          REQUIRE_NOTHROW(opt = MoveTracker<T>(std::move(opt).unwrap()));

          counter->valid(expected);
        }

        expected.moves += 2;
        REQUIRE_NOTHROW(value = std::move(opt).unwrap());
        counter->valid(expected);
      }
    });
  }
}

// vim:

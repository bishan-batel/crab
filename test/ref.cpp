#include "ref.hpp"
#include <catch2/catch_test_macros.hpp>
#include "test_static_asserts.hpp"
#include "test_types.hpp"

TEST_CASE("Type Traits") {
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

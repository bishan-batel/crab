#include "ref.hpp"
#include <catch2/catch_test_macros.hpp>
#include "crab/type_traits.hpp"
#include "test_static_asserts.hpp"
#include "test_types.hpp"

struct IncompleteType;

TEST_CASE("Type Traits") {

  STATIC_REQUIRE(not crab::complete_type<IncompleteType>);

  assert::for_types(assert::common_types, []<typename T>(assert::type<T>) {
    STATIC_REQUIRE(crab::complete_type<String>);
  });
}

struct IncompleteType {};

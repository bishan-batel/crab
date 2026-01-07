#include <catch2/catch_test_macros.hpp>
#include <crab/fn.hpp>
#include <tuple>
#include <optional>
#include "crab/mem/move.hpp"
#include "test_static_asserts.hpp"
#include "test_types.hpp"

using namespace crab;

TEST_CASE("mem::move") {

  STATIC_CHECK(requires(Copyable copyable) {
    { mem::move(copyable) } -> ty::same_as<Copyable&&>;
  });
}

TEST_CASE("mem::address_of") {
  assert::for_types(assert::common_types, []<typename T>(assert::type<T>) {
    T v1{};
    const T v2{};

    STATIC_CHECK(std::addressof(v1) == mem::address_of(v1));
    STATIC_CHECK(std::addressof(v2) == mem::address_of(v2));

    CHECK(std::addressof(v1) == mem::address_of(v1));
    CHECK(std::addressof(v2) == mem::address_of(v2));
  });
}

TEST_CASE("mem::size_of") {
  assert::for_types(assert::common_types, []<typename T>(assert::type<T>) {
    INFO(typeid(T).name());
    STATIC_CHECK(sizeof(T) == mem::size_of<T>());
  });
}

TEST_CASE("mem::swap") {

  SECTION("integers") {
    i32 a = 1;
    i32 b = 2;

    mem::swap(a, b);

    CHECK(a == 2);
    CHECK(b == 1);

    mem::swap<i32>(a, b);

    CHECK(a == 1);
    CHECK(b == 2);
  }

  SECTION("move tracking") {
    MoveCount e1{}, e2{};
    RcMut<MoveCount> c1{make_rc_mut<MoveCount>()}, c2{make_rc_mut<MoveCount>()};

    MoveTracker<Copyable> m1{MoveTracker<Copyable>::from(c1)};
    m1.inner().set_name("one");

    MoveTracker<Copyable> m2{MoveTracker<Copyable>::from(c2)};
    m2.inner().set_name("two");

    c1->valid(e1);
    c2->valid(e2);

    m1.operator=(m1);
    m2.operator=(m2);

    c1->valid(e1);
    c2->valid(e2);

    CHECK(m1.inner().get_name() == "one");
    CHECK(m2.inner().get_name() == "two");

    mem::swap(c1, c2);

    CHECK(m1.inner().get_name() == "two");
    CHECK(m2.inner().get_name() == "one");

    e1.moves += 2;
    e2.moves += 1;

    c1->valid(e1);
    c2->valid(e2);
  }
}

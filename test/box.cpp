
#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <crab/preamble.hpp>
#include <crab/box.hpp>
#include <utility>
#include "test_types.hpp"

struct SelfReferential;

struct SelfReferential {
  Option<Box<SelfReferential>> test;
};

struct Type {};

void fun(i32&) {
  std::cout << "fun1" << std::endl;
}

void fun(i32&&) {
  std::cout << "fun2" << std::endl;
}

class EpicBacon {
public:

  explicit EpicBacon(i32 x): number{new i32{x}} {}

  EpicBacon(const EpicBacon& from): number{new i32{*from.number}} {}

  EpicBacon(EpicBacon&& from) noexcept: number{from.number} {
    from.number = nullptr;
  }

  ~EpicBacon() {
    delete number;
  }

  i32* number;
};

EpicBacon some_function() {
  return EpicBacon{42};
}

void another_function(EpicBacon e) {
  std::cout << e.number << "\n";
  EpicBacon{std::move(e)};
}

void epic() {
  EpicBacon resource{some_function()};

  // do something ...

  another_function(std::move(resource));
}

void box() {
  Box<Derived> derived{crab::make_box<Derived>()};

  // this would not compile, you need to move / transfer ownership
  // Derived* raw_ptr = Box<Derived>::unwrap(derived);

  // this would not compile, you need to move / transfer ownership
  Derived* raw_ptr = Box<Derived>::unwrap(std::move(derived));

  delete raw_ptr;
}

void unique() {
  std::unique_ptr<Derived> derived{std::make_unique<Derived>()};

  Derived* a = derived.release();

  delete a;
}

TEST_CASE("Preamble", "[unit]") {
  REQUIRE(unit{} == unit{});
  REQUIRE(unit::val == unit{});
}

TEST_CASE("Box", "[box]") {
  SECTION("Box Moving") {
    Box<u32> a = crab::make_box<u32>(10);
    CHECK_NOTHROW(a.as_ptr() != nullptr);

    const Box<u32> moved = std::move(a);
    std::cout << moved << std::endl;
    CHECK_THROWS(a.as_ptr() == nullptr);
    CHECK_THROWS(a.as_ptr());
    CHECK_NOTHROW(moved.as_ptr());

    CHECK(*moved == 10);
    CHECK(moved == 10);
  }

  SECTION("Const Correctness") {
    const Box<u32> var = crab::make_box<u32>(42);
    const u32* ptr = var.as_ptr();
    REQUIRE(*var == 42);
    REQUIRE(*ptr == 42);
  }

  SECTION("Releasing Single") {
    Box<u32> single = crab::make_box<u32>(420);
    u32* raw_ptr = single.as_ptr();
    REQUIRE(raw_ptr != nullptr);
    REQUIRE(*raw_ptr == 420);

    u32* b;
    REQUIRE_NOTHROW(b = Box<u32>::unwrap(std::move(single)));

    REQUIRE(b == raw_ptr);

    REQUIRE(b != nullptr);

    REQUIRE(*b == 420);

    REQUIRE_THROWS(Box<u32>::unwrap(std::move(single)));

    // Generics<G>
    delete b;
  }
}

TEST_CASE("Box Option Niche Optimization") {
  Option<Box<i32>> opt{crab::make_box<i32>(10)};

  static_assert(sizeof(Option<Box<i32>>) == sizeof(Box<i32>));

  REQUIRE(opt.is_some());
  REQUIRE_NOTHROW(opt.get_unchecked());
  REQUIRE_NOTHROW(*opt.get_unchecked() == 10);

  Box<int>& a{opt.get_unchecked()};
  REQUIRE_NOTHROW(std::move(a));

  Option moved_into{std::move(opt)};
  CHECK(moved_into.is_some());

  CHECK_THROWS(opt.get_unchecked().as_ptr());
  CHECK(opt.is_none());
  CHECK(not opt.is_some());

  opt = crab::none;
  CHECK(opt.is_none());
}

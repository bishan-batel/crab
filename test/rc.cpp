//
// Created by bishan_ on 5/20/24.
//

#include "rc.hpp"

#include <catch2/catch_test_macros.hpp>

struct Huh {
  i32 v;

  // ReSharper disable once CppMemberFunctionMayBeStatic
  auto num() -> i32 { return 42; }
};

struct Bruh : Huh {
  explicit Bruh(const i32 v) : Huh{v} {}
};

TEST_CASE("Rc") {
  SECTION("String") {
    Rc<String> a = crab::make_rc<String>("what");
  }
  SECTION("Downcast") {
    Rc<Bruh> original = crab::make_rc<Bruh>(42);

    const Rc<Huh> huh = original.upcast<Huh>();
    const Rc<Huh> huh1 = original;

    Option<Rc<Bruh>> returned = original.downcast<Bruh>();

    REQUIRE(huh->v == 42);

    REQUIRE(returned.is_some());
    REQUIRE(crab::unwrap(std::move(returned))->v == 42);
  }
}

#pragma once

#include "crab/core.hpp"
#include "crab/ty/classify.hpp"

namespace crab::boxed {
  template<typename T>
  class Box;

  namespace impl {
    template<typename T>
    struct BoxStorage;
  }

  template<ty::complete_type T, typename... Args>
  requires std::constructible_from<T, Args...>
  [[nodiscard]] CRAB_INLINE constexpr static auto make_box(Args&&... args) -> Box<T>;

}

namespace crab::prelude {
  using boxed::Box;
}

CRAB_PRELUDE_GUARD;

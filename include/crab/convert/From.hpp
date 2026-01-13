#pragma once

#include <concepts>

#include "crab/mem/forward.hpp"

namespace crab::conv {

  namespace impl {
    template<typename T, typename F>
    concept has_from_method = requires {
      { T::from(std::declval<F>()) } -> ty::same_as<T>;
    };
  }

  template<typename T, typename F>
  concept From = std::convertible_to<F, T> or impl::has_from_method<T, F>;

  template<typename T, typename F>
  requires From<T, F>
  [[nodiscard]] constexpr auto from(F&& value) -> T {
    if constexpr (impl::has_from_method<T, F>) {
      return T::from(mem::forward<F>(value));
    } else {
      return static_cast<T>(mem::forward<F>(value));
    }
  }

}

namespace crab {
  using crab::conv::from;
}

#pragma once
#include <utility>
#include "ref.hpp"

namespace crab::fn {
  constexpr auto identity = [](auto&& x) { return x; };

  constexpr auto constant = [](auto&& x) {
    return [x = std::forward<decltype(x)>(x)]<typename... Args>(Args&&...) {
      return x;
    };
  };

  template<typename T, typename R, typename... Args>
  constexpr auto method(R (T::*method)(Args&&...)) {
    return [method](auto&& x, Args&&... args) {
      RefMut<T> ref{x};
      return ((*ref).*method)(std::forward<Args>(args)...);
    };
  }

}

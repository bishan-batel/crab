#pragma once
#include <preamble.hpp>
#include <utility>
#include "box.hpp"
#include "rc.hpp"
#include "ref.hpp"

namespace crab::fn {
  /**
   * @brief Identity Function, f(x)=x forall x
   */
  constexpr auto identity = []<typename T>(T&& x) -> T {
    static_assert(
      std::move_constructible<T>,
      "Cannot create an identity function for a type that cannot be moved."
    );
    return std::forward<T>(x);
  };

  /**
   * Takes in some value x and returns a function that maps any input (and any
   * number of inputs) to x
   *
   * @param x Any integer value to check
   */
  constexpr auto constant = []<typename T>(T&& x) {
    static_assert(
      std::move_constructible<T>,
      "Cannot create a constant function for a type that cannot be moved."
    );
    return [x = std::forward<T>(x)]<typename... Args>(Args&&...) {
      return decltype(x){x};
    };
  };

  /**
   * Predicate for whether the input is even
   */
  constexpr auto is_even = [](const auto& x) -> bool { return x % 2 == 0; };

  /**
   * Predicate for whether the input is odd
   */
  constexpr auto is_odd = [](const auto& x) -> bool { return not is_even(x); };

  template<typename Derived>
  struct cast_s final {
    [[nodiscard]] constexpr auto operator()(auto& value
    ) const -> Option<RefMut<Derived>>
      requires(not std::is_const_v<decltype(value)>)
    {
      return crab::ref::cast<Derived>(value);
    }

    [[nodiscard]] constexpr auto operator()(const auto& value
    ) const -> Option<Ref<Derived>> {
      return crab::ref::cast<Derived>(value);
    }

    template<typename T>
    [[nodiscard]] constexpr auto operator()(RefMut<T> value
    ) const -> Option<RefMut<Derived>> {
      return operator()(*value);
    }

    template<typename T>
    [[nodiscard]] constexpr auto operator()(Ref<T> value
    ) const -> Option<Ref<Derived>> {
      return operator()(*value);
    }

    template<typename T>
    [[nodiscard]] constexpr auto operator()(RcMut<T> value
    ) const -> Option<RcMut<Derived>> {
      return value.template downcast<T>();
    }

    template<typename T>
    [[nodiscard]] constexpr auto operator()(Rc<T> value
    ) const -> Option<Rc<Derived>> {
      return value.template downcast<T>();
    }

    template<typename T>
    [[nodiscard]] constexpr auto operator()(const Box<T>& value
    ) const -> Option<Ref<Derived>> {
      return operator()(*value);
    }

    template<typename T>
    [[nodiscard]] constexpr auto operator()(Box<T>& value
    ) const -> Option<RefMut<Derived>> {
      return operator()(*value);
    }
  };

  // a particle needs to keep track of its lifetime
  // its all in the same compute shader
  // because when a lifetime restarts it needs to reset it

  /**
   * @brief Function Object that is basically the same thing as
   * crab::ref::cast<T>
   *
   * The only use of this versus the former is when in use of Function
   * Programming, such as with std::ranges, for when you need to pass
   * functions around as objects
   */
  template<typename Derived>
  inline static constexpr auto cast = cast_s<Derived>{};

  template<typename T, typename R, typename... Args>
  [[nodiscard]] constexpr auto method(R (T::*method)(Args&&...)) {
    return [method](auto&& x, Args&&... args) {
      RefMut<T> ref{x};
      return ((*ref).*method)(std::forward<Args>(args)...);
    };
  }

  /// -------------------------------------------------------------------------
  /// Range Functions

}

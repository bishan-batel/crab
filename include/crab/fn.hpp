#pragma once

#include <concepts>
#include <utility>

#include <crab/preamble.hpp>
#include <crab/box.hpp>
#include <crab/ref.hpp>
#include "crab/rc/Rc.hpp"

namespace crab::fn {
  /**
   * @brief Identity Function, f(x)=x forall x
   */
  constexpr auto identity{
    []<typename T>(T&& x) {
      static_assert(std::move_constructible<T>, "Cannot create an identity function for a type that cannot be moved.");
      return std::forward<T>(x);
    },
  };

  /**
   * Takes in some value x and returns a function that maps any input (and any
   * number of inputs) to x
   *
   * @param x Any integer value to check
   */
  constexpr auto constant{
    []<ty::copy_constructible T>(T x) { return [x = std::move(x)]<typename... Args>(Args&&...) -> T { return x; }; },
  };

  /**
   * Predicate for whether the input is even
   */
  constexpr auto is_even{
    [](auto&& x) -> bool { return x % 2 == 0; },
  };

  /**
   * Predicate for whether the input is odd
   */
  constexpr auto is_odd{
    [](auto&& x) -> bool { return not is_even(x); },
  };

  template<typename Derived>
  struct cast_s final {

    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(auto& value) const requires ty::non_const<decltype(value)>
    {
      return ref::cast<Derived>(value);
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(const auto& value) const {
      return ref::cast<Derived>(value);
    }

    template<typename T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(RefMut<T> value) const {
      return ref::cast<Derived>(value);
    }

    template<typename T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(Ref<T> value) const -> opt::Option<Ref<Derived>> {
      return ref::cast<Derived>(value);
    }

    template<typename T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(RcMut<T> value) const {
      return value.template downcast<T>();
    }

    template<typename T>
    [[nodiscard]]
    inline constexpr auto operator()(Rc<T> value) const -> opt::Option<Rc<Derived>> {
      return value.template downcast<T>();
    }

    template<typename T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(const Box<T>& value) const -> opt::Option<const Derived&> {
      return operator()(*value);
    }

    template<typename T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(Box<T>& value) const -> opt::Option<Derived&> {
      return operator()(*value);
    }
  };

  /**
   * @brief Function Object that is basically the same thing as
   * crab::ref::cast<T>
   *
   * The only use of this versus the former is when in use of Function
   * Programming, such as with std::ranges, for when you need to pass
   * functions around as objects
   */
  template<typename Derived>
  inline static constexpr cast_s<Derived> cast{};
}

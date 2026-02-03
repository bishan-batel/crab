/// @file crab/fn/cast.hpp

#pragma once

#include "crab/core.hpp"
#include "crab/rc/Rc.hpp"
#include "crab/ref/casts.hpp"
#include "crab/ref/ref.hpp"

namespace crab::fn {

  /// Functor type for use with crab::fn::cast
  /// @ingroup fn
  template<typename Derived>
  struct cast_s final {

    [[nodiscard]] CRAB_INLINE constexpr auto operator()(auto& value) const requires ty::non_const<decltype(value)>
    {
      return ref::cast<Derived>(value);
    }

    [[nodiscard]] CRAB_INLINE constexpr auto operator()(const auto& value) const {
      return ref::cast<Derived>(value);
    }

    template<typename T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(ref::RefMut<T> value) const {
      return ref::cast<Derived>(value);
    }

    template<typename T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(ref::Ref<T> value) const -> opt::Option<ref::Ref<Derived>> {
      return ref::cast<Derived>(value);
    }

    template<typename T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(rc::RcMut<T> value) const {
      return value.template downcast<T>();
    }

    template<typename T>
    [[nodiscard]]
    inline constexpr auto operator()(Rc<T> value) const -> opt::Option<Rc<Derived>> {
      return value.template downcast<T>();
    }

    template<typename T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(const boxed::Box<T>& value) const
      -> opt::Option<const Derived&> {
      return operator()(*value);
    }

    template<typename T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(boxed::Box<T>& value) const -> opt::Option<Derived&> {
      return operator()(*value);
    }
  };

  /// Function Object that is basically the same thing as crab::ref::cast<T>
  ///
  /// The only use of this versus the former is when in use of Function
  /// Programming, such as with std::ranges, for when you need to pass
  /// functions around as objects.
  ///
  /// For more information, refer to `crab::ref::cast`
  ///
  /// @ingroup fn
  template<typename Derived>
  inline static constexpr cast_s<Derived> cast{};
}

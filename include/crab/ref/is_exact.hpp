/// @file crab/ref/is_exact.hpp
/// @ingroup ref

#pragma once

#include <typeinfo>
#include "crab/core.hpp"

namespace crab::ref {

  /// @ingroup ref
  ///
  /// Is this given parameter *exactly* this type.
  ///
  /// This will not perform a recursive check like dynamic_cast. This symbol is also exposed as simply crab::is_exact
  ///
  /// # Examples
  ///
  /// ```cpp
  /// class A {};
  ///
  /// class B: public A {};
  ///
  /// class C: public B {};
  ///
  /// i32 main() {
  ///   A a;
  ///   B b;
  ///
  ///   crab_check(crab::ref::is_exact<A>(a));
  ///   crab_check(crab::ref::is_exact<B>(b));
  ///
  ///   crab_check(crab::ref::is<A>(c));
  ///   crab_check(not crab::ref::is_exact<A>(c));
  /// }
  /// ```
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto is_exact([[maybe_unused]] const auto& obj) noexcept -> bool {
    return typeid(obj) == typeid(T);
  }
}

namespace crab {
  using ref::is_exact;
}

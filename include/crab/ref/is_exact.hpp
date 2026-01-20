#pragma once

#include <typeinfo>
#include "crab/core.hpp"

namespace crab::ref {

  /**
   * @brief Is this given parameter *exactly* this type.
   *
   * This will not perform a recursive check like dynamic_cast
   *
   * ```cpp
   *
   * class A {};
   *
   * class B: public A {};
   *
   * class C: public B {};
   *
   * i32 main() {
   *   A a;
   *   B b;
   *
   *   debug_assert(crab::ref::is_exact<A>(a),"");
   *   debug_assert(crab::ref::is_exact<B>(b),"");
   *
   *   debug_assert(crab::ref::is<A>(c),"");
   *   debug_assert(not crab::ref::is_exact<A>(c),"");
   * }
   *
   *
   * ```
   *
   * @tparam Derived
   * @param obj
   * @return
   */
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto is_exact(const auto& obj) noexcept -> bool {
    return typeid(obj) == typeid(T);
  }
}

namespace crab {
  using crab::ref::is_exact;
}

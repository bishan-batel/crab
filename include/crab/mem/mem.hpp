#include <type_traits>
#include "./preamble.hpp"
#include "./type_traits.hpp"
#include "crab/core.hpp"

// required for std::addressof on MSVC, no public API like __builtin_addressof
#if CRAB_MSVC_VERSION
#include <memory>
#endif

namespace crab {

  namespace mem {

    template<typename T>
    concept trivially_reloctable = std::is_trivially_copyable_v<T>;

    template<ty::non_const T, ty::convertible<T> U>
    CRAB_PURE_INLINE_CONSTEXPR auto replace(T& dest, U&& value) -> T {
      T original{move(dest)};

      T&& value_possibly_converted{value};

      dest = move(value_possibly_converted);

      return original;
    }

    namespace impl {
      template<ty::non_const T>
      CRAB_CONSTEXPR auto swap_non_trivial(T& lhs, T& rhs) -> void {
        T&& temp{move(lhs)};
        lhs = move(rhs);
        rhs = move(temp);

        // TODO:
      }

      template<ty::non_const T>
      CRAB_CONSTEXPR auto swap_trivial_relocatable(T& lhs, T& rhs) -> void {
        T&& temp{move(lhs)};
        lhs = move(rhs);
        rhs = move(temp);
      }
    }

    // clang-format on

    template<ty::non_const T>
    CRAB_CONSTEXPR auto swap_nonoverlapping(T& __restrict lhs, T& __restrict rhs) -> void {
      if constexpr (not trivially_reloctable<T>) {
        impl::swap_non_trivial(lhs, rhs);
        return;
      }

      if (std::is_constant_evaluated()) {
        impl::swap_non_trivial(lhs, rhs);
      } else {
        impl::swap_trivial_relocatable(lhs, rhs);
      }
    }

    template<ty::non_const T>
    CRAB_CONSTEXPR auto swap(T& lhs, T& rhs) noexcept(
      std::is_nothrow_move_assignable_v<T> and std::is_nothrow_move_constructible_v<T>
    ) -> void {
      if constexpr (not trivially_reloctable<T>) {
        impl::swap_non_trivial(lhs, rhs);
        return;
      }

      if (std::is_constant_evaluated()) {
        impl::swap_non_trivial(lhs, rhs);
      } else {
        if (address_of(lhs) == address_of(rhs)) {
          CRAB_LIKELY {
            impl::swap_trivial_relocatable(lhs, rhs);
          }
        }
      }
    }

  }

  using mem::move;
}

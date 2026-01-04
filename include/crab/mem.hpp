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
    struct is_trivially_reloctable : std::false_type {};

    template<typename T>
    requires std::is_trivially_copyable_v<T>
    struct is_trivially_reloctable<T> : std::true_type {};

    template<typename T>
    concept trivially_reloctable = is_trivially_reloctable<T>::value;

    /**
     * Equivalent to std::move with the added constraint of not being able to move from const, which is almost always a
     * bug
     */
    template<typename T>
    CRAB_PURE_INLINE_CONSTEXPR auto move(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
      -> decltype(auto) {

      using TBase = ty::remove_reference<T>;
      static_assert(ty::non_const<TBase>, "Cannot move from a const value");

      return static_cast<TBase&&>(value);
    }

    /**
     * Crab version of sizeof(T) that disallows reference types, using reference types with sizeof is almost always a
     * bug
     */
    template<ty::non_reference T>
    CRAB_PURE_INLINE_CONSTEXPR auto size_of() -> usize {
      static_assert(ty::non_reference<T>, "Cannot get the sizeof a reference.");
      return sizeof(T);
    }

    /**
     * Returns the size of a given value's type
     */
    template<typename T>
    CRAB_PURE_INLINE_CONSTEXPR auto size_of_val(const T&) -> usize {
      return size_of<T>();
    }

    /**
     * Retrieves the address / pointer to the given value, bypassing any operator& overloads
     */
    template<typename T>
    CRAB_PURE_INLINE_CONSTEXPR auto address_of(T& value) -> T* {
#if CRAB_GCC_VERSION || CRAB_CLANG_VERSION
      return __builtin_addressof(value);
#else
      std::addressof(value);
#endif
    }

    template<typename T>
    CRAB_PURE_INLINE_CONSTEXPR auto address_of(T&& value) -> T* = delete;

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
      CRAB_CONSTEXPR auto swap_trivial(T& lhs, T& rhs) -> void {
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
        impl::swap_trivial(lhs, rhs);
      } else {
        impl::swap_non_trivial(lhs, rhs);
      }
    }

    template<ty::non_const T>
    CRAB_CONSTEXPR auto swap(T& lhs, T& rhs) -> void {
      // clang-format off
      if (address_of(lhs) == address_of(rhs)) CRAB_UNLIKELY {
        return;
      }
      // clang-format on

      return swap_nonoverlapping(lhs, rhs);
    }

  }

  using mem::move;
}

/// @file crab/result/Err.hpp
/// @ingroup result
#pragma once

#include "crab/core.hpp"
#include "crab/mem/address_of.hpp"
#include "crab/mem/move.hpp"
#include "crab/mem/forward.hpp"
#include "crab/result/concepts.hpp"

namespace crab {
  namespace result {

    /// Thin wrapper / tagged type over a value to be given to Result::Result(Err) (constructor)
    template<typename E>
    struct [[nodiscard]] Err final {
      static_assert(error_type<E>, "Err<E> must satisfy ok_type");

      using Inner = E;

      CRAB_INLINE constexpr explicit Err(E value): value(mem::move(value)) {}

      [[nodiscard]] CRAB_INLINE constexpr auto get() & -> E& {
        return value;
      }

      [[nodiscard]] CRAB_INLINE constexpr auto get() const& -> const E& {
        return value;
      }

      [[nodiscard]] CRAB_INLINE constexpr auto get() && -> E&& {
        return mem::move(value);
      }

    private:

      E value;
    };

    /// @copydoc Err
    /// Specialisation to allow reference error types.
    template<typename E>
    struct [[nodiscard]] Err<E&> final {
      using Inner = E&;

      CRAB_INLINE constexpr explicit Err(E& value): ptr{mem::address_of(value)} {}

      [[nodiscard]] CRAB_INLINE constexpr auto get() const -> E& {
        return *ptr;
      }

    private:

      E* ptr;
    };

    /// Helper to construct an error type E wrapped in crab::result::Err. The benefit of this over simply construction E
    /// is when you want to guarentee that the value produced will be accepted as the 'error' overload for Result.
    template<error_type E, typename... Args>
    [[nodiscard]] CRAB_INLINE constexpr auto err(Args&&... args) -> Err<E> {
      return Err<E>{E(mem::forward<Args>(args)...)};
    }

    /// Wraps the given error type in a Err, this is less helpful than the other overload.
    template<typename E>
    [[nodiscard]] CRAB_INLINE constexpr auto err(E value) -> Err<E> {

      static_assert(error_type<E>, "Value must be a possible 'Err<E>' type for use in Result<T, E>");

      return Err<E>{mem::move(value)};
    }

  }

  using result::Err;
  using result::err;
}

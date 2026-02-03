#pragma once

#include "crab/core.hpp"
#include "crab/mem/address_of.hpp"
#include "crab/mem/move.hpp"
#include "crab/mem/forward.hpp"
#include "crab/result/concepts.hpp"

namespace crab {
  namespace result {

    /**
     * @brief Thin wrapper / tagged type over a value to be given to Result<T,E>(Err)'s
     * constructor
     */
    template<typename E>
    struct Err {
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
        return mem::forward<E>(value);
      }

    private:

      E value;
    };

    template<typename E>
    struct Err<E&> {
      using Inner = E&;

      CRAB_INLINE constexpr explicit Err(E& value): ptr{mem::address_of(value)} {}

      [[nodiscard]] CRAB_INLINE constexpr auto get() const -> E& {
        return *ptr;
      }

    private:

      E* ptr;
    };

    template<error_type E, typename... Args>
    [[nodiscard]] CRAB_INLINE constexpr auto err(Args&&... args) {
      return Err<E>{E(mem::forward<Args>(args)...)};
    }

    template<typename E>
    [[nodiscard]] CRAB_INLINE constexpr auto err(E value) {

      static_assert(ok_type<E>, "Value must be a possible 'Err<E>' type for use in Result<T, E>");

      return Err<E>{mem::move(value)};
    }

  }

  using result::Err;
  using result::err;
}

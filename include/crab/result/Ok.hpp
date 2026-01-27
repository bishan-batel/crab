#pragma once

#include "crab/core.hpp"
#include "crab/mem/address_of.hpp"
#include "crab/mem/move.hpp"
#include "crab/mem/forward.hpp"
#include "crab/result/concepts.hpp"

namespace crab {

  namespace result {

    /**
     * @brief Thin wrapper over a value to be given to Result<T,E>(Ok)'s
     * constructor
     */
    template<typename T>
    struct Ok {
      static_assert(ok_type<T>, "Ok<T> must satisfy ok_type");

      using Inner = T;

      CRAB_INLINE constexpr explicit Ok(T value): value(mem::move(value)) {}

      [[nodiscard]] CRAB_INLINE constexpr auto get() & -> T& {
        return value;
      }

      [[nodiscard]] CRAB_INLINE constexpr auto get() const& -> const T& {
        return value;
      }

      [[nodiscard]] CRAB_INLINE constexpr auto get() && -> T&& {
        return mem::forward<T>(value);
      }

    private:

      T value;
    };

    template<typename T>
    struct Ok<T&> {
      using Inner = T&;

      CRAB_INLINE constexpr explicit Ok(T& value): ptr{mem::address_of(value)} {}

      [[nodiscard]] CRAB_INLINE constexpr auto get() const -> T& {
        return *ptr;
      }

    private:

      T* ptr;
    };

    template<ok_type T, typename... Args>
    [[nodiscard]] CRAB_INLINE constexpr auto ok(Args&&... args) {
      return Ok<T>{T{mem::forward<Args>(args)...}};
    }

    template<typename T>
    [[nodiscard]] CRAB_INLINE constexpr auto ok(T value) {
      static_assert(ok_type<T>, "Value must be a possible 'Ok<T>' type for use in Result<T, E>");

      return Ok<T>{mem::move(value)};
    }

  }

  using result::Ok;
  using result::ok;
}

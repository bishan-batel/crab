#pragma once

#include "crab/core.hpp"
#include "crab/mem/take.hpp"
#include "crab/opt/none.hpp"

namespace crab::opt::impl {
  /**
   * @brief Specialized storage for Option<T&> to not require an in_use flag
   */
  template<typename T>
  struct RefStorage {
    CRAB_INLINE constexpr explicit RefStorage(T& value): inner{&value} {}

    CRAB_INLINE constexpr explicit RefStorage(const None& = {}): inner{nullptr} {}

    CRAB_INLINE constexpr RefStorage(const RefStorage& from) = default;

    CRAB_INLINE constexpr RefStorage(RefStorage&& from) noexcept {
      inner = mem::take(from.inner);
    }

    CRAB_INLINE constexpr RefStorage& operator=(const RefStorage& from) = default;

    CRAB_INLINE constexpr RefStorage& operator=(RefStorage&& from) = default;

    CRAB_INLINE constexpr ~RefStorage() = default;

    CRAB_INLINE constexpr auto operator=(T& value) -> RefStorage& {
      inner = &value;
      return *this;
    }

    CRAB_INLINE constexpr auto operator=(const None&) -> RefStorage& {
      inner = nullptr;
      return *this;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto value() const& -> T& {
      return *inner;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto value() & -> T& {
      return *inner;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto value() && -> T& {
      return *mem::take(inner);
    }

    [[nodiscard]] CRAB_INLINE constexpr auto in_use() const -> bool {
      return inner != nullptr;
    }

  private:

    T* inner;
  };

}

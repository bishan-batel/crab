#pragma once

#include "crab/core.hpp"
#include "crab/mem/take.hpp"
#include "crab/option/none.hpp"

namespace crab::opt::impl {
  /**
   * @brief Specialized storage for Option<T&> to not require an in_use flag
   */
  template<typename T>
  struct RefStorage {
    CRAB_INLINE_CONSTEXPR explicit RefStorage(T& value): inner{&value} {}

    CRAB_INLINE_CONSTEXPR explicit RefStorage(const None& = {}): inner{nullptr} {}

    CRAB_INLINE_CONSTEXPR RefStorage(const RefStorage& from) = default;

    CRAB_INLINE_CONSTEXPR RefStorage(RefStorage&& from) noexcept {
      inner = mem::take(from.inner);
    }

    CRAB_INLINE_CONSTEXPR RefStorage& operator=(const RefStorage& from) = default;

    CRAB_INLINE_CONSTEXPR RefStorage& operator=(RefStorage&& from) = default;

    CRAB_INLINE_CONSTEXPR ~RefStorage() = default;

    CRAB_INLINE_CONSTEXPR auto operator=(T& value) -> RefStorage& {
      inner = &value;
      return *this;
    }

    CRAB_INLINE_CONSTEXPR auto operator=(const None&) -> RefStorage& {
      inner = nullptr;
      return *this;
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto value() const& -> T& {
      return *inner;
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto value() & -> T& {
      return *inner;
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto value() && -> T& {
      return *mem::take(inner);
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto in_use() const -> bool {
      return inner != nullptr;
    }

  private:

    T* inner;
  };

}

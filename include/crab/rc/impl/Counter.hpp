#pragma once

#include "crab/assertion/check.hpp"
#include "crab/core/SourceLocation.hpp"
#include "crab/ty/bool_types.hpp"

#include <atomic>

namespace crab::rc::impl {
  class Counter final {

  public:

    explicit CRAB_INLINE Counter(const usize strong_count = 1, const usize weak_count = 0):
        strong{strong_count}, weak{weak_count} {}

    constexpr CRAB_INLINE auto increment_strong() -> void {
      strong++;
    }

    constexpr CRAB_INLINE auto increment_weak() -> void {
      weak++;
    }

    [[nodiscard]] constexpr CRAB_INLINE auto has_any_strong() const -> bool {
      return strong != 0;
    }

    [[nodiscard]] constexpr CRAB_INLINE auto has_any_weak() const -> bool {
      return weak != 0;
    }

    [[nodiscard]] constexpr CRAB_INLINE auto strong_count() const -> usize {
      return strong;
    }

    [[nodiscard]] constexpr CRAB_INLINE auto weak_count() const -> usize {
      return weak;
    }

    /**
     * Decrements the strong reference counter
     *
     * Returns whether this leaves the counter at 0
     */
    [[nodiscard]]
    constexpr CRAB_INLINE auto decrement_strong(const SourceLocation& loc = SourceLocation::current()) -> bool {
      crab_dbg_check_with_location(strong != 0, loc, "Counter::decrement should not cause unsigned underflow");

      strong--;

      return strong == 0;
    }

    /**
     * Decrements the weak reference counter
     *
     * Returns whether this leaves the counter at 0
     */
    [[nodiscard]]
    constexpr CRAB_INLINE auto decrement_weak(const SourceLocation& loc = SourceLocation::current()) -> bool {
      crab_dbg_check_with_location(strong != 0, loc, "Counter::decrement should not cause unsigned underflow");

      weak--;

      return weak == 0;
    }

  private:

    static constexpr bool IsAtomic = false;
    ty::conditional<IsAtomic, std::atomic_size_t, usize> strong{}, weak{};
  };

}

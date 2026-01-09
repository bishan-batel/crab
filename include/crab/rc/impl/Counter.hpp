#pragma once

#include "crab/preamble.hpp"
#include "crab/type_traits.hpp"

#include <atomic>

namespace crab::rc::impl {
  template<bool IsAtomic>
  class Counter {

  public:

    explicit Counter(usize strong_count = 1, usize weak_count = 0): strong{strong_count}, weak{weak_count} {}

    constexpr auto increment_strong() -> void {
      strong++;
    }

    constexpr auto increment_weak() -> void {
      weak++;
    }

    [[nodiscard]] constexpr auto has_any_strong() const -> bool {
      return strong != 0;
    }

    [[nodiscard]] constexpr auto has_any_weak() const -> bool {
      return weak != 0;
    }

    [[nodiscard]] constexpr auto strong_count() const -> usize {
      return strong;
    }

    [[nodiscard]] constexpr auto weak_count() const -> usize {
      return weak;
    }

    /**
     * Decrements the strong reference counter
     *
     * Returns whether this leaves the counter at 0
     */
    [[nodiscard]] constexpr auto decrement_strong(const SourceLocation& loc = SourceLocation::current()) -> bool {
      debug_assert_transparent(strong != 0, loc, "Counter::decrement should not cause unsigned underflow");

      return --strong == 0;
    }

    /**
     * Decrements the weak reference counter
     *
     * Returns whether this leaves the counter at 0
     */
    [[nodiscard]] constexpr auto decrement_weak(const SourceLocation& loc = SourceLocation::current()) -> bool {
      debug_assert_transparent(strong != 0, loc, "Counter::decrement should not cause unsigned underflow");

      return --weak == 0;
    }

  private:

    ty::conditional<IsAtomic, std::atomic_size_t, usize> strong{}, weak{};
  };

}

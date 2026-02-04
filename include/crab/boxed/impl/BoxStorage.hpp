
#pragma once

#include "crab/boxed/forward.hpp"
#include "crab/core/discard.hpp"
#include "crab/mem/move.hpp"
#include "crab/opt/none.hpp"
#include "crab/assertion/check.hpp"

namespace crab::boxed::impl {

  /// Storage container specialization for Option<Box<T>>
  /// @internal
  template<typename T>
  struct BoxStorage final {
    using Box = crab::boxed::Box<T>;

    CRAB_INLINE constexpr explicit BoxStorage(Box value): inner{mem::move(value)} {}

    CRAB_INLINE constexpr explicit BoxStorage(const opt::None& = {}): inner{nullptr} {}

    CRAB_INLINE constexpr auto operator=(Box&& value) -> BoxStorage& {
      crab_check(value.obj != nullptr, "Option<Box<T>>, BoxStorage::operator= called with an invalid box");
      inner = mem::move(value);
      return *this;
    }

    CRAB_INLINE constexpr auto operator=(const opt::None&) -> BoxStorage& {
      crab::discard(Box{mem::move(inner)});
      return *this;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto value() const& -> const Box& {
      return inner;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto value() & -> Box& {
      return inner;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto value() && -> Box {
      return mem::move(inner);
    }

    [[nodiscard]] CRAB_INLINE constexpr auto in_use() const -> bool {
      return inner.obj != nullptr;
    }

  private:

    Box inner;
  };
}

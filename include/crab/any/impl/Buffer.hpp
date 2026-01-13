#pragma once

#include <array>
#include <cstddef>
#include "crab/core.hpp"
#include "crab/num/integer.hpp"

namespace crab::any::impl {

  template<usize Size, usize Alignment>
  struct Buffer final {

    template<typename T>
    CRAB_RETURNS_NONNULL CRAB_PURE auto as_ptr() const -> const T* {
      return reinterpret_cast<const T*>(buffer.data());
    }

    template<typename T>
    CRAB_RETURNS_NONNULL CRAB_PURE auto as_ptr() -> T* {
      return reinterpret_cast<T*>(buffer.data());
    }

    alignas(Alignment) std::array<std::byte, Size> buffer;
  };
}

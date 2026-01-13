#pragma once

#include <array>
#include <cstddef>
#include <memory>

#include "./Buffer.hpp"

namespace crab::any::impl {

  template<typename T>
  struct AnyOfStorage final {
    template<usize Size, usize Align, typename... Args>
    requires std::constructible_from<T, Args...>
    static auto construct(Buffer<Size, Align>& buffer, Args&&... args) -> void {
      std::construct_at<T, Args...>(as_ptr(buffer), std::forward<Args>(args)...);
    }

    template<usize Size, usize Align>
    static auto destroy(Buffer<Size, Align>& buffer) -> void {
      std::destroy_at(as_ptr(buffer));
    }

    template<usize Size, usize Align>
    CRAB_PURE static auto as_ptr(Buffer<Size, Align>& buffer) -> T* {
      return buffer.template as_ptr<T>();
    }

    template<usize Size, usize Align>
    CRAB_PURE static auto as_ptr(const Buffer<Size, Align>& buffer) -> const T* {
      return buffer.template as_ptr<T>();
    }

    template<usize Size, usize Align>
    CRAB_PURE static auto as_ref(Buffer<Size, Align>& buffer) -> T& {
      return *as_ptr(buffer);
    }

    template<usize Size, usize Align>
    CRAB_PURE static auto as_ref(const Buffer<Size, Align>& buffer) -> const T& {
      return *as_ptr(buffer);
    }
  };
}

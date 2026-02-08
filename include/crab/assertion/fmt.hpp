#pragma once

#include "crab/core.hpp"
#include "crab/core/cases.hpp"
#include "crab/str/str.hpp"

#include <concepts>
#include <sstream>

// solution to fmtlib complaining on windows
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS // NOLINT

#if CRAB_CLANG_VERSION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-literal-operator"
#endif

#if CRAB_GCC_VERSION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-reference"
#endif

#include <fmt/format.h>

#if CRAB_GCC_VERSION
#pragma GCC diagnostic pop
#endif

#if CRAB_CLANG_VERSION
#pragma clang diagnostic pop
#endif

namespace crab {
  template<typename T>
  [[nodiscard]] auto builtin_to_string(T&& obj) -> String {
    return crab::cases{
      []<std::integral Ty>(Ty&& x) { return std::to_string(x); },
      []<std::floating_point Ty>(Ty&& x) { return std::to_string(x); },
      [](const bool x) { return String{x ? "true" : "false"}; },
      [](String str) { return str; },
      [](auto&& x) -> String {
        constexpr bool pipeable = requires(std::ostringstream stream) { stream << std::declval<T&&>(); };

        if constexpr (pipeable) {
          std::ostringstream stream{};
          stream << x;
          return std::move(stream).str();
        } else {
          return typeid(T).name();
        }
      }
    }(obj);
  }

  template<typename T>
  [[nodiscard]] constexpr auto to_string(T&& obj) -> String {
    if constexpr (requires(const T& obj) { fmt::to_string(std::forward<T>(obj)); }) {
      return fmt::to_string(std::forward<T>(obj));
    } else {
      return builtin_to_string<T>(std::forward<T>(obj));
    }
  }

  [[nodiscard]] constexpr auto format() -> String {
    return {};
  }

  using fmt::format;
}

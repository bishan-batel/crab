#pragma once

#include "crab/preamble.hpp"
#include <concepts>
#include <string>

// Enum for what format library crab will use
#define CRAB_FMT_USAGE_STD           0
#define CRAB_FMT_USAGE_FMTLIB        1
#define CRAB_FMT_USAGE_COMPATABILITY 2

// Determining support for std::format
#if CRAB_HAS_INCLUDE("format")
#define CRAB_SUPPORTS_STD_FORMAT true
#else
#define CRAB_SUPPORTS_STD_FORMAT false
#endif

// Determine support for fmt::format
#if CRAB_HAS_INCLUDE("fmt/format.h")
#define CRAB_SUPPORTS_FMTLIB true
#else
#define CRAB_SUPPORTS_FMTLIB false
#endif

// Automatically select what formatter to use
#ifndef CRAB_FMT_USAGE

// rely on std::format, else use compatability
#if CRAB_SUPPORTS_STD_FORMAT
#define CRAB_FMT_USAGE CRAB_FMT_USAGE_STD
#else
#define CRAB_FMT_USAGE CRAB_FMT_USAGE_COMPATABILITY
#endif

#endif

// Picking what formatter to use

#if CRAB_FMT_USAGE == CRAB_FMT_USAGE_STD

#if CRAB_SUPPORTS_STD_FORMAT
#include <format>
#else
#error                                                                                                                 \
  "Compiler does not support the use of std::format, consider using CRAB_FMT_USAGE=1 to use fmtlib or compatability mode"
#endif

#elif CRAB_FMT_USAGE == CRAB_FMT_USAGE_FMTLIB

#if CRAB_SUPPORTS_FMTLIB

#if CRAB_CLANG_VERSION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-literal-operator"
#endif

#include <fmt/format.h>

#if CRAB_CLANG_VERSION
#pragma clang diagnostic pop
#endif

#else
#error "CRAB_USE_FMT_LIB is true but fmtlib is not available."
#endif

#elif CRAB_FMT_USAGE == CRAB_FMT_USAGE_COMPATABILITY

#else

#error                                                                                                                 \
  "Macro CRAB_FMT_USAGE_COMPATABILITY set to an invalid number, must either be 0 (std::format), 1 (fmt::format), or 2 (compatability)"

#endif

namespace crab {
  template<typename T>
  CRAB_NODISCARD auto builtin_to_string(T&& obj) -> String {
    return crab::cases{
      []<std::integral Ty>(Ty&& x) { return std::to_string(x); },
      []<std::floating_point Ty>(Ty&& x) { return std::to_string(x); },
      [](const bool x) { return String{x ? "true" : "false"}; },
      [](String str) { return str; },
      [](auto&& x) -> String {
        constexpr bool pipeable = requires(std::stringstream stream) { stream << std::declval<T&&>(); };

        if constexpr (pipeable) {
          std::stringstream stream{};
          stream << x;
          return std::move(stream).str();
        } else {
          return typeid(T).name();
        }
      }
    }(obj);
  }

#if CRAB_FMT_USAGE == CRAB_FMT_USAGE_FMTLIB

  template<typename T>
  [[nodiscard]] constexpr auto to_string(T&& obj) -> String {
    if constexpr (requires(const T& obj) { fmt::to_string(std::forward<T>(obj)); }) {
      return fmt::to_string(std::forward<T>(obj));
    } else {
      return builtin_to_string<T>(std::forward<T>(obj));
    }
  }
#else
  template<typename T>
  [[nodiscard]] constexpr auto to_string(T&& obj) -> String {
    return builtin_to_string(std::forward<T>(obj));
  }
#endif

#if CRAB_FMT_USAGE == CRAB_FMT_USAGE_STD

  using std::format;

#elif CRAB_FMT_USAGE == CRAB_FMT_USAGE_FMTLIB

  using fmt::format;

#else
  namespace helper::fmt {}

  template<typename... Args>
  CRAB_NODISCARD auto format(auto&& fmt, Args&&... args) -> String {
    std::stringstream stream{};
    stream << "FORMAT_STRING(\"";
    stream << fmt;
    stream << "\") % (";
    (..., (stream << ", " << to_string(args)));
    stream << ')';

    return std::move(stream).str();
  }
#endif
}

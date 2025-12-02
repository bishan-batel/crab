#pragma once

#include "preamble.hpp"

// Enum for what format library crab will use
#define CRAB_FMT_USAGE_STD           0
#define CRAB_FMT_USAGE_FMTLIB        1
#define CRAB_FMT_USAGE_COMPATABILITY 2

// Determining support for std::format
#if __has_include("format")
#define CRAB_SUPPORTS_STD_FORMAT true
#else
#define CRAB_SUPPORTS_STD_FORMAT false
#endif

// Determine support for fmt::format
#if __has_include("fmt/format.h")
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
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/xchar.h>
#else
#error "CRAB_USE_FMT_LIB is true but fmtlib is not available."
#endif

#elif CRAB_FMT_USAGE == CRAB_FMT_USAGE_COMPATABILITY

#else

#error                                                                                                                 \
  "Macro CRAB_FMT_USAGE_COMPATABILITY set to an invalid number, must either be 0 (std::format), 1 (fmt::format), or 2 (compatability)"
#endif

#define CRAB_FORMAT()

namespace crab {

#if CRAB_FMT_USAGE == CRAB_FMT_USAGE_STD
  template<typename... Args>
  CRAB_PURE_INLINE_CONSTEXPR auto format(std::format_string<Args...>&& fmt, Args&&... args) -> decltype(auto) {
    return std::format(std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
  }
#elif CRAB_FMT_USAGE == CRAB_FMT_USAGE_FMTLIB
  template<typename... Args>
  CRAB_PURE_INLINE_CONSTEXPR auto format(fmt::format_string<Args...>&& fmt, Args&&... args) -> decltype(auto) {
    return fmt::format(std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
  }
#elif CRAB_FMT_USAGE == CRAB_FMT_USAGE_COMPATABILITY
  template<typename... Args>
  CRAB_PURE_INLINE_CONSTEXPR auto format(fmt::format_string<Args...>&& fmt, Args&&... args) -> String {
    return fmt::format(std::forward<fmt::format_string<Args...>>(fmt), std::forward<Args>(args)...);
  }
#endif
}

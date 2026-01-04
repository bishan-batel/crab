#pragma once

/**
 * Current version of crab's API
 */

#include "crab/config.hpp"

#define CRAB_USE_PRELUDE true

#if NDEBUG
#define CRAB_DEBUG   0
#define CRAB_RELEASE 1
#else
#define CRAB_DEBUG   1
#define CRAB_RELEASE 0
#endif

/// ===================================================================================================================
///                                                 Preproc & Compile-Time Helpers
/// ===================================================================================================================

#if defined(__clang__)
#define CRAB_CLANG_VERSION (__clang_major__ * 100 + __clang_minor__)
#else
#define CRAB_CLANG_VERSION 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define CRAB_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#define CRAB_GCC_VERSION 0
#endif

#ifdef _MSVC_VER
#define CRAB_MSVC_VERSION _MSC_VER
#else
#define CRAB_MSVC_VERSION 0
#endif

#ifdef _MSVC_LANG
#define CRAB_CPP_VERSION _MSVC_LANG
#else
#define CRAB_CPP_VERSION __cplusplus
#endif

#ifdef __has_feature
#define CRAB_HAS_FEATURE(x) __has_feature(x)
#else
#define CRABCRAB_HAS_FEATURE(x) false
#endif

#ifdef __has_include
#define CRAB_HAS_INCLUDE(x) __has_include(x)
#else
#define CRAB_HASCRAB_HAS_INCLUDE(x) 0
#endif

#ifdef __has_cpp_attribute
#define CRAB_HAS_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define CRAB_HAS_ATTRIBUTE(x) 0
#endif

#define CRAB_PRAGMA_(...)      _Pragma(#__VA_ARGS__)
#define CRAB_PRAGMA(...)       CRAB_PRAGMA_(__VA_ARGS__)

#define CRAB_STRINGIFY(...)    #__VA_ARGS__

#define CRAB_DEFER(macro, ...) macro(__VA_ARGS__)

#if CRAB_CLANG_VERSION || CRAB_GCC_VERSION
#define CRAB_WARNING(msg) CRAB_PRAGMA(GCC warning msg)
#else
#define CRAB_WARNING(msg) CRAB_PRAGMA(message msg)
#endif

/// ===================================================================================================================
///                                                 Function Annotations
/// ===================================================================================================================

#if CRAB_GCC_VERSION || CRAB_CLANG_VERSION
#define CRAB_INLINE inline __attribute__((always_inline))
#else
#define CRAB_INLINE inline
#endif

#if CRAB_HAS_ATTRIBUTE(noreturn)
#define CRAB_NORETURN [[noreturn]]
#else
#define CRAB_NORETURN
#endif

#if CRAB_HAS_ATTRIBUTE(nodiscard)
#define CRAB_NODISCARD       [[nodiscard]]
#define CRAB_NODISCARDF(msg) [[nodiscard(msg)]]
#else
#define CRAB_NODISCARD
#define CRAB_NODISCARDF(msg)
#endif

#if CRAB_HAS_ATTRIBUTE(maybe_unused)
#define CRAB_MAYBE_UNUSED [[maybe_unused]]
#else
#define CRAB_MAYBE_UNUSED
#endif

#if CRAB_HAS_ATTRIBUTE(gnu::may_alias)
#define CRAB_MAY_ALIAS [[gnu::may_alias]]
#else
#define CRAB_MAY_ALIAS
#endif

#if CRAB_HAS_ATTRIBUTE(likely)
#define CRAB_LIKELY [[likely]]
#else
#define CRAB_LIKELY
#endif

#if CRAB_HAS_ATTRIBUTE(unlikely)
#define CRAB_UNLIKELY [[unlikely]]
#else
#define CRAB_UNLIKELY
#endif

#define CRAB_CONSTEXPR constexpr

#if ((CRAB_GCC_VERSION >= 1000 || CRAB_CLANG_VERSION >= 1101)                                                          \
     && (!defined(__apple_build_version__) || __apple_build_version__ >= 14000029L) && CRAB_CPLUSPLUS >= 202002L)      \
  || (defined(__cpp_consteval) && (!CRAB_MSVC_VERSION || CRAB_MSVC_VERSION >= 1929))
#define CRAB_CONSTEVAL consteval
#else
#define CRAB_CONSTEVAL CRAB_CONSTEXPR
#endif

#if CRAB_GCC_VERSION || CRAB_CLANG_VERSION
#define CRAB_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
#define CRAB_RETURNS_NONNULL
#endif

#define CRAB_INLINE_CONSTEXPR      CRAB_INLINE CRAB_CONSTEXPR

#define CRAB_PURE_INLINE_CONSTEXPR CRAB_NODISCARD CRAB_INLINE CRAB_CONSTEXPR

#define CRAB_PURE_CONSTEVAL        CRAB_NODISCARD CRAB_CONSTEVAL

#define CRAB_PURE_CONSTEXPR        CRAB_NODISCARD CRAB_CONSTEXPR

/// ===================================================================================================================
///                                                 Optimisation Controls
/// ===================================================================================================================

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable <= CRAB_CPLUSPLUS
#include <utility>
#define CRAB_HAS_UNREACHABLE true
#else
#define CRAB_HAS_UNREACHABLE false
#endif

#if CRAB_MSVC_VERSION && !CRAB_CLANG_VERSION
#define CRAB_ASSUME(condition) __assume(static_cast<bool>(condition))
#elif CRAB_CLANG_VERSION
#define CRAB_ASSUME(condition) __builtin_assume(static_cast<bool>(condition))
#elif CRAB_GCC_VERSION

#if CRAB_HAS_ATTRIBUTE(assume)
#define CRAB_ASSUME(condition)                                                                                         \
  do { __attribute__((assume(static_cast<bool>(condition))))} while(false)
#else
#define CRAB_ASSUME(condition)                                                                                         \
  do {                                                                                                                 \
    if (not static_cast<bool>(condition)) {                                                                            \
      __builtin_unreachable();                                                                                         \
    }                                                                                                                  \
  } while (false)
#endif

#else

#define CRAB_ASSUME(condition)                                                                                         \
  do {                                                                                                                 \
    if (not static_cast<bool>(condition)) {                                                                            \
      while (true);                                                                                                    \
    }                                                                                                                  \
  } while (false)

#endif

namespace crab {

  /**
   * @brief Denotes unreachable paths
   * This should be used for optimisation purposes only.
   */
  CRAB_NORETURN CRAB_INLINE auto unreachable() -> void {
#if CRAB_HAS_UNREACHABLE
    std::unreachable();
#elif CRAB_MSVC_VERSION && !CRAB_CLANG_VERSION
    CRAB_ASSUME(false);
#elif CRAB_CLANG_VERSION || CRAB_GCC_VERSION
    __builtin_unreachable();
#else
    CRAB_ASSUME(false);
    while (true);
  }
#endif
  }

  /**
   * Used to discard / explicitly ignore certain outputs
   */
  CRAB_INLINE_CONSTEXPR auto discard(CRAB_MAYBE_UNUSED auto&&...) -> void {}
}

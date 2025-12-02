#pragma once

/**
 * Current version of crab's API
 */
#define CRAB_VERSION 200100

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

#ifdef _MSC_VER
#define CRAB_MSC_VERSION _MSC_VER
#else
#define CRAB_MSC_VERSION 0
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

#if CRAB_HAS_ATTRIBUTE(nodiscard)
#define CRAB_NODISCARD       [[nodiscard]]
#define CRAB_NODISCARDF(msg) [[nodiscard(msg)]]
#else
#define CRAB_NODISCARD
#define CRAB_NODISCARDF(msg)
#endif

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

#define CRAB_CONSTEXPR constexpr

#if ((CRAB_GCC_VERSION >= 1000 || CRAB_CLANG_VERSION >= 1101)                                                          \
     && (!defined(__apple_build_version__) || __apple_build_version__ >= 14000029L) && CRAB_CPLUSPLUS >= 202002L)      \
  || (defined(__cpp_consteval) && (!CRAB_MSC_VERSION || CRAB_MSC_VERSION >= 1929))
#define CRAB_CONSTEVAL consteval
#else
#define CRAB_CONSTEVAL CRAB_CONSTEXPR
#endif

#define CRAB_PURE_FN               CRAB_NODISCARD

#define CRAB_INLINE_CONSTEXPR      CRAB_INLINE CRAB_CONSTEXPR

#define CRAB_PURE_INLINE_CONSTEXPR CRAB_PURE_FN CRAB_INLINE CRAB_CONSTEXPR

#define CRAB_PURE_CONSTEVAL        CRAB_PURE_FN CRAB_CONSTEVAL

#define CRAB_PURE_CONSTEXPR        CRAB_PURE_FN CRAB_CONSTEXPR

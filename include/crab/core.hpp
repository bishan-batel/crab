/// @file crab/core.hpp
/// @brief Crab's core macros used in every other header.
/// This mainly includes compiler-related macros as well as the basis for options (such as CRAB_USE_PRELUDE)

#pragma once

/// @defgroup core Core
/// @{

#include "crab/config.hpp"

/// @def CRAB_USE_PRELUDE
/// @brief Option for whether or not crab should 'using namespace crab::prelude'
///
/// If this macro is defined, then the global namespace will include
/// symbols from crab::prelude (eg. using namespace crab::prelude).
///
/// By default, this macro is set to 'true' unless defined otherwise.

#ifndef CRAB_USE_PRELUDE
#define CRAB_USE_PRELUDE true
#endif

/// @def CRAB_DEBUG
/// @hideinitializer
/// @brief Defined to be 1 or 0 depending on whether this is a debug build or not
/// Alias for '!NDEBUG', note that CRAB_DEBUG=1 means that CRAB_RELEASE=0 (and vice versa)

/// @def CRAB_RELEASE
/// @hideinitializer
/// @brief Defined to be 1 or 0 depending on whether this is a release build or not
/// Alias for 'NDEBUG', note that CRAB_RELEASE=1 means that CRAB_DEBUG=0 (and vice versa)

#if NDEBUG
#define CRAB_DEBUG   0
#define CRAB_RELEASE 1
#else
#define CRAB_DEBUG   1
#define CRAB_RELEASE 0
#endif

/// @def CRAB_OSX
/// @hideinitializer
/// @brief Defined when compiling for apple (mac/ios) targets

#if defined(__APPLE__) && __APPLE__
#define CRAB_OSX __APPLE__
#else
#define CRAB_OSX 0
#endif

/// @def CRAB_WIN32
/// @hideinitializer
/// @brief Defined when compiling for Windows targets

#if defined(_WIN32) && _WIN32
#define CRAB_WIN32 _WIN32
#else
#define CRAB_WIN32 0
#endif

/// @def CRAB_LINUX
/// @hideinitializer
/// @brief Defined when compiling for linux targets

#if defined(__linux__) && __linux__
#define CRAB_LINUX __linux__
#else
#define CRAB_LINUX 0
#endif

/// @def CRAB_UNIX
/// @hideinitializer
/// @brief Defined when compiling for unix-compatible targets (eg. linux OR apple)

#if CRAB_LINUX || CRAB_OSX
#define CRAB_UNIX 1
#else
#define CRAB_UNIX 0
#endif

/// @def CRAB_CLANG_VERSION
/// @hideinitializer
/// @brief Numeric representation of the clang version being compiled with (0 if not being compiled with clang)

#if defined(__clang__)
#define CRAB_CLANG_VERSION (__clang_major__ * 100 + __clang_minor__)
#else
#define CRAB_CLANG_VERSION 0
#endif

/// @def CRAB_GCC_VERSION
/// @hideinitializer
/// @brief Numeric representation of the GCC version being compiled with (0 if not being compiled with GCC)

#if defined(__GNUC__) && !defined(__clang__)
#define CRAB_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#else
#define CRAB_GCC_VERSION 0
#endif

/// @def CRAB_MSVC_VERSION
/// @hideinitializer
/// @brief Numeric representation of the MSVC version being compiled with (0 if not being compiled with MSVC)

#ifdef _MSC_VER
#define CRAB_MSVC_VERSION _MSC_VER
#else
#define CRAB_MSVC_VERSION 0
#endif

/// @def CRAB_CPP_VERSION
/// @hideinitializer
/// @brief Numeric representation (standardized) of the current C++ version being compiled for

#ifdef _MSVC_LANG
#define CRAB_CPP_VERSION _MSVC_LANG
#else
#define CRAB_CPP_VERSION __cplusplus
#endif

/// @def CRAB_HAS_FEATURE(feature)
/// @hideinitializer
/// @brief Evalutes to whether the current compiler & standard supports a given feature
/// Alias of __has_feature unless that macro is not valid

#ifdef __has_feature
#define CRAB_HAS_FEATURE(feature) __has_feature(feature)
#else
#define CRAB_HAS_FEATURE(feature) false
#endif

/// @def CRAB_HAS_INCLUDE(include_path)
/// @hideinitializer
/// @brief Evaluates to whether or not the path given would be a valid include path

#ifdef __has_include
#define CRAB_HAS_INCLUDE(include_path) __has_include(include_path)
#else
#define CRAB_HASCRAB_HAS_INCLUDE(x) 0
#endif

/// @def CRAB_HAS_ATTRIBUTE(attr_name)
/// @hideinitializer
/// @brief Evaluates to whether or not the given C++ attribute is recognized by the current compiler.
///
/// For example, [[likely]] can be checked to exist by CRAB_HAS_ATTRIBUTE(likely)

#ifdef __has_cpp_attribute
#define CRAB_HAS_ATTRIBUTE(attr_name) __has_cpp_attribute(attr_name)
#else
#define CRAB_HAS_ATTRIBUTE(x) 0
#endif

/// @internal
/// Helper macro for CRAB_PRAGMA
#define CRAB_PRAGMA_(...) _Pragma(#__VA_ARGS__)

/// Helper macro for invoking _Pragma with a non-stringified source.
/// This macro will stringify all inputs and pass it into _Pragma
/// @hideinitializer
#define CRAB_PRAGMA(...) CRAB_PRAGMA_(__VA_ARGS__)

/// Helper macro that simply stringifies the input.
#define CRAB_STRINGIFY(...) #__VA_ARGS__

/// Helper macro to apply a macro to given arguments, useful for deferring evaluation.
/// For example, CRAB_DEFER(CRAB_STRINGIFY, hi) -> CRAB_STRINGIFY(hi) -> "hi"
#define CRAB_DEFER(macro, ...) macro(__VA_ARGS__)

/// @def CRAB_WARNING(msg)
/// @hideinitializer
/// Function-Like (#pragma) macro for emitting a compiler warning.

#if CRAB_CLANG_VERSION || CRAB_GCC_VERSION
#define CRAB_WARNING(msg) CRAB_PRAGMA(GCC warning msg)
#else
#define CRAB_WARNING(msg) CRAB_PRAGMA(message msg)
#endif

/// @def CRAB_INLINE
/// @hideinitializer
/// Annotation for functions to force compilers to inline.

#if CRAB_GCC_VERSION || CRAB_CLANG_VERSION
#define CRAB_INLINE inline __attribute__((always_inline))
#else
#define CRAB_INLINE inline
#endif

/// @def CRAB_MAY_ALIAS
/// @hideinitializer
/// Annotation for fields to hint at aliasing. This is GNU specific.
///
/// Full Documentation on
/// [GCC's Website](https://gcc.gnu.org/onlinedocs/gcc-4.6.3/gcc/Type-Attributes.html#i386%20Type%20Attributes)
#if CRAB_HAS_ATTRIBUTE(gnu::may_alias)
#define CRAB_MAY_ALIAS [[gnu::may_alias]]
#else
#define CRAB_MAY_ALIAS
#endif

/// @def CRAB_CONSTEVAL
/// @hideinitializer
/// Alias for 'consteval'. However, this will default to constexpr on compilers that do not support consteval.

#if ((CRAB_GCC_VERSION >= 1000 || CRAB_CLANG_VERSION >= 1101)                                                          \
     && (!defined(__apple_build_version__) || __apple_build_version__ >= 14000029L) && CRAB_CPLUSPLUS >= 202002L)      \
  || (defined(__cpp_consteval) && (!CRAB_MSVC_VERSION || CRAB_MSVC_VERSION >= 1929))
#define CRAB_CONSTEVAL consteval
#else
#define CRAB_CONSTEVAL constexpr
#endif

/// @def crab_if_consteval
/// Macro for branching depending on if operating in a constant evaluated context or not. This is used if you are
/// writing in a space that may or may not be in C++23 with 'if consteval'.
///
/// # Examples
/// ```cpp
/// template<typename T>
/// constexpr auto copy(const T& from, T& to) {
///   crab_if_consteval() {
///     to = from;
///     return;
///   }
///
///   if constexpr (std::is_trivially_copy_assignable_v<T>) {
//      memcpy(crab::mem::address_of(to), crab::mem::address_of(from), sizeof(T));
//      return;
///   }
///
///   to = from;
///
/// }
///
/// ```

#if __cpp_if_consteval
#define crab_if_consteval if consteval
#else
#include <type_traits>
#define crab_if_consteval if (::std::is_constant_evaluated())
#endif

/// @def CRAB_RETURNS_NONNULL
/// @hideinitializer
/// GCC Annotation for a Function to indicate that its return type is never null.
/// This macro has no effect on platforms that do not support `returns_nonnull`

#if CRAB_HAS_ATTRIBUTE(returns_nonnull)
#define CRAB_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
#define CRAB_RETURNS_NONNULL
#endif

/// @def CRAB_PURE
/// @hideinitializer
/// Annotation for a function to indicate it is 'pure'.
/// A pure function is one that does not depend on global state nor modifies global state. Note this macro will also add
/// the `[[nodiscard]]` annotation, as a pure function with no return value is meaningless.

#if CRAB_HAS_ATTRIBUTE(pure)
#define CRAB_PURE [[nodiscard]] __attribute__((pure))
#else
#define CRAB_PURE [[nodiscard]]
#endif

/// @internal
/// @hideinitializer
/// @def CRAB_HAS_UNREACHABLE
/// Internal annotation for crab to check if std::unreachable is valid or not

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable <= CRAB_CPLUSPLUS
#include <utility>
#define CRAB_HAS_UNREACHABLE true
#else
#define CRAB_HAS_UNREACHABLE false
#endif

/// @def CRAB_ASSUME(condition)
/// @hideinitializer
/// Macro to tell the compiler an assumption it can make about the current state.

#if CRAB_MSVC_VERSION && !CRAB_CLANG_VERSION
#define CRAB_ASSUME(condition) __assume(static_cast<bool>(condition))
#elif CRAB_CLANG_VERSION
#define CRAB_ASSUME(condition) __builtin_assume(static_cast<bool>(condition))
#elif CRAB_GCC_VERSION

#if CRAB_HAS_ATTRIBUTE(assume)
#define CRAB_ASSUME(condition)                                                                                         \
  do {                                                                                                                 \
    __attribute__((assume(static_cast<bool>(condition))));                                                             \
  } while (false)
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

/// Helper to defer preprocessor evaluation when doing macro jank.
#define CRAB_EVAL0(...) __VA_ARGS__

/// Helper to defer preprocessor evaluation when doing macro jank.
#define CRAB_EVAL1(...) CRAB_EVAL0(CRAB_EVAL0(CRAB_EVAL0(__VA_ARGS__)))

/// Helper to defer preprocessor evaluation when doing macro jank.
#define CRAB_EVAL2(...) CRAB_EVAL1(CRAB_EVAL1(CRAB_EVAL1(__VA_ARGS__)))

/// Helper to defer preprocessor evaluation when doing macro jank.
#define CRAB_EVAL3(...) CRAB_EVAL2(CRAB_EVAL2(CRAB_EVAL2(__VA_ARGS__)))

/// Helper to defer preprocessor evaluation when doing macro jank.
#define CRAB_EVAL4(...) CRAB_EVAL3(CRAB_EVAL3(CRAB_EVAL3(__VA_ARGS__)))

/// Helper to defer preprocessor evaluation when doing macro jank.
#define CRAB_EVAL(...) CRAB_EVAL4(CRAB_EVAL4(CRAB_EVAL4(__VA_ARGS__)))

/// @internal
/// @def CRAB_PRELUDE_GUARD
/// Crab-internal helper expansion for using the crab::prelude if the settings apply.

#if CRAB_USE_PRELUDE
#define CRAB_PRELUDE_GUARD using namespace crab::prelude
#else
#define CRAB_PRELUDE_GUARD
#endif

/// @namespace crab::prelude
/// @brief Namespace that includes the default things crab puts in the global namespace.
///
/// This includes things like Option<T>, Box<T>, Rc<T>, etc.
/// If you want to disable this behavior, explicitly define CRAB_USE_PRELUDE to false.
namespace crab::prelude {}

CRAB_PRELUDE_GUARD;

/// }@

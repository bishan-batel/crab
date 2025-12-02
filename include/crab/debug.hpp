//
// Created by bishan_ on 4/22/24.
//

#pragma once

#include "./preamble.hpp"
#include "./core.hpp"

#if _DEBUG

#include <source_location>
#include "./fmt.hpp"

namespace crab::debug {
  class AssertionFailedError final : public std::exception {
    String fmt;

  public:

    explicit AssertionFailedError(SourceLocation source_location, StringView assertion_text, String msg):
        fmt{crab::format(
          "Failed Assertion in:\n {}:{}: in {} \n'{}'\n{}\n{}",
          source_location.file_name(),
          source_location.line(),
          source_location.column(),
          source_location.function_name(),
          assertion_text,
          msg
        )} {}

    CRAB_NODISCARD auto what() const noexcept -> const char* override {
      return fmt.c_str();
    }
  };

  CRAB_NORETURN CRAB_INLINE unit dbg_assert(SourceLocation source_location, StringView assertion_text, String msg) {
    throw AssertionFailedError{source_location, assertion_text, std::move(msg)};
  }

} // namespace crab::debug

/**
 * Asserts that the given condition is true when compiled in debug mode, if not then this will halt the program &
 * print the given error.
 *
 * This is a slightly more complicated version of `debug_assert`, the difference being this requires an explicit
 * SourceLocation to be passed in for the error message, which is useful when using asserts in a a nested library
 * definition.
 *
 * The first argument is the runtime expression that must evaluate to true, then second is the C++20
 * fmt::format format string for the error message. The following arguments are format args akin to passing in
 * fmt::format(fmt_string, a1, a2, ...)
 */
#define debug_assert_transparent(condition, source_location, ...)                                                      \
  if (!static_cast<bool>(condition)) do {                                                                              \
      crab::debug::dbg_assert(source_location, #condition, crab::format(__VA_ARGS__));                                 \
  } while (false)

/**
 * Asserts that the given condition is true when compiled in debug mode, if not then this will halt the program &
 * print the given error.
 *
 * The first argument is the runtime expression that must evaluate to true, then second is the C++20
 * fmt::format format string for the error message. The following arguments are format args akin to passing in
 * fmt::format(fmt_string, a1, a2, ...)
 */
#define debug_assert(condition, ...) debug_assert_transparent(condition, SourceLocation::current(), __VA_ARGS__)

#else

#define debug_assert_transparent(condition, source_location, ...)                                                      \
  do {                                                                                                                 \
    std::ignore = source_location;                                                                                     \
  } while (false)

#define debug_assert(...)

#endif

//
// Created by bishan_ on 4/22/24.
//

#pragma once

#include <crab/preamble.hpp>

#include <source_location>
#include <fmt/format.h>

#if _DEBUG

namespace crab::debug {
  class AssertionFailedError final : public std::exception {
    String fmt;

  public:

    explicit AssertionFailedError(SourceLocation source_location, StringView assertion_text, String msg):
        fmt{fmt::format(
          "Failed Assertion in:\n {}:{}: in {} \n'{}'\n{}\n{}",
          source_location.file_name(),
          source_location.line(),
          source_location.column(),
          source_location.function_name(),
          assertion_text,
          msg
        )} {}

    [[nodiscard]] auto what() const noexcept -> const char* override {
      return fmt.c_str();
    }
  };

  [[noreturn]] inline unit dbg_assert(SourceLocation source_location, StringView assertion_text, String msg) {
    throw AssertionFailedError{source_location, assertion_text, msg};
  }

} // namespace crab::debug

/**
 * Asserts that the given condition is true when compiled in debug mode, if not then this will halt the program & print
 * the given error.
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
      crab::debug::dbg_assert(source_location, #condition, fmt::format(__VA_ARGS__));                                  \
  } while (false)

/**
 * Asserts that the given condition is true when compiled in debug mode, if not then this will halt the program & print
 * the given error.
 *
 * The first argument is the runtime expression that must evaluate to true, then second is the C++20
 * fmt::format format string for the error message. The following arguments are format args akin to passing in
 * fmt::format(fmt_string, a1, a2, ...)
 */
#define debug_assert(condition, ...) debug_assert_transparent(condition, SourceLocation::current(), __VA_ARGS__)

#else

#define debug_assert_transparent(condition, source_location, ...)                                                      \
  while (false) {                                                                                                      \
    std::ignore = source_location;                                                                                     \
  }

#define debug_assert(...)

#endif

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

    explicit AssertionFailedError(
      std::source_location source_location,
      StringView assertion_text,
      String msg
    ):
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

  [[noreturn]] inline unit dbg_assert(
    std::source_location source_location,
    StringView assertion_text,
    String msg
  ) {
    throw AssertionFailedError{source_location, assertion_text, msg};
  }

} // namespace crab::debug

#define debug_assert_transparent(condition, source_location, ...)              \
  if (!static_cast<bool>(condition)) do {                                      \
      crab::debug::dbg_assert(                                                 \
        source_location,                                                       \
        #condition,                                                            \
        fmt::format(__VA_ARGS__)                                               \
      );                                                                       \
  } while (false)

#define debug_assert(condition, ...)                                           \
  debug_assert_transparent(                                                    \
    condition,                                                                 \
    std::source_location::current(),                                           \
    __VA_ARGS__                                                                \
  )

#else

#define debug_assert_transparent(condition, source_location, ...)              \
  while (false) {                                                              \
    std::ignore = source_location;                                             \
  }

#define debug_assert(...)

#endif

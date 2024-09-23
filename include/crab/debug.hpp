//
// Created by bishan_ on 4/22/24.
//

#pragma once
#include <format>
#include <source_location>
#include "../preamble.hpp"

namespace crab::debug {
  class AssertionFailedError final : public std::exception {
    std::string fmt;

  public:
    explicit AssertionFailedError(std::source_location source_location, StringView assertion_text, StringView msg) :
        /* fmt{std::format("Failed Assertion in:\n {}:{} in {} \n'{}'\n{}", source, line, function, assertion_text,
           msg)} { */
        fmt{std::format(
            "Failed Assertion in:\n {}:{}: in {} \n'{}'\n{}",
            source_location.file_name(),
            source_location.line(),
            source_location.column(),
            source_location.function_name(),
            assertion_text,
            msg)} {}

    ~AssertionFailedError() override = default;

    [[nodiscard]] auto what() const noexcept -> const char * final { return fmt.c_str(); }
  };

  inline unit dbg_assert(std::source_location source_location, StringView assertion_text, StringView msg) {
    throw AssertionFailedError{source_location, assertion_text, msg};
  }
} // namespace crab::debug
  //
#if DEBUG
  #define debug_assert_transparent(condition, message, source_location)                                                \
    if (!static_cast<bool>(condition)) do {                                                                            \
        crab::debug::dbg_assert(source_location, #condition, (message));                                               \
    } while (false)

  #define debug_assert(condition, message) debug_assert_transparent(condition, message, std::source_location::current())
#else

  #define debug_assert_transparent(...)
  #define debug_assert(...)

#endif

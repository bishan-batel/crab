//
// Created by bishan_ on 4/22/24.
//
#include "../include/crab/debug.hpp"

#include <format>

namespace crab::debug {
  AssertionFailedError::AssertionFailedError(
    StringView function,
    StringView source,
    StringView assertion_text,
    usize line,
    StringView msg
  )
    : fmt{
      std::format(
        "Failed Assertion in:\n {}:{} in {} \n'{}'\n{}",
        source,
        line,
        function,
        assertion_text,
        msg
      )
    } {}

  AssertionFailedError::~AssertionFailedError() = default;

  const char *AssertionFailedError::what() const noexcept {
    return fmt.c_str();
  }

  unit dbg_assert(
    [[maybe_unused]] const StringView function,
    [[maybe_unused]] const StringView source,
    [[maybe_unused]] const StringView assertion_line,
    [[maybe_unused]] const usize line,
    [[maybe_unused]] const StringView msg
  ) {
    throw AssertionFailedError{function, source, assertion_line, line, msg};;
  }
}

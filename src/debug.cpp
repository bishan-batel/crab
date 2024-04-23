//
// Created by bishan_ on 4/22/24.
//
#include "debug.hpp"
#include "box.hpp"
#include "option.hpp"
#include "preamble.hpp"
#include "range.hpp"
#include "rc.hpp"
#include "ref.hpp"

#include <format>

namespace crab::debug {
  AssertionFailedError::AssertionFailedError(
    StringView function,
    StringView source,
    StringView assertion_text,
    usize line,
    StringView msg

  ) : fmt{std::format("Failed Assertion in:\n {}:{} in {} \n'{}'\n{}", source, line, function, assertion_text, msg)} {}

  AssertionFailedError::~AssertionFailedError() = default;

  const char *AssertionFailedError::what() const noexcept {
    return fmt.c_str();
  }

  [[noreturn]]
  void dbg_assert(
    const bool succeeded,
    const StringView function,
    const StringView source,
    const StringView assertion_line,
    const usize line,
    const StringView msg
  ) {
    if (succeeded) return;

    const AssertionFailedError error{function, source, assertion_line, line, msg};

    #if DEBUG
    throw error;
    #else
    std::std::cerr << error << std::endl;
    #endif
  }
}

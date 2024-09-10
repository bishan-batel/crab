//
// Created by bishan_ on 4/22/24.
//

#pragma once
#include <format>
#include "../preamble.hpp"

namespace crab::debug {
  class AssertionFailedError final : public std::exception {
    std::string fmt;

  public:
    explicit AssertionFailedError(
        StringView function, StringView source, StringView assertion_text, usize line, StringView msg) :
        fmt{std::format("Failed Assertion in:\n {}:{} in {} \n'{}'\n{}", source, line, function, assertion_text, msg)} {
    }

    ~AssertionFailedError() override = default;

    [[nodiscard]] auto what() const noexcept -> const char * final { return fmt.c_str(); }
  };

  inline unit dbg_assert(StringView function, StringView source, StringView assertion_line, usize line, StringView msg) {
    throw AssertionFailedError{function, source, assertion_line, line, msg};
  }
} // namespace crab::debug
  //
#if DEBUG
  #define debug_assert(condition, message)                                                                             \
    if (!static_cast<bool>(condition)) crab::debug::dbg_assert(__FUNCTION__, __FILE__, #condition, __LINE__, (message))
#else

  #define debug_assert(...)

#endif

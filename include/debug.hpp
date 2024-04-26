//
// Created by bishan_ on 4/22/24.
//

#pragma once
#include "preamble.hpp"

#if DEBUG

namespace crab::debug {
  class AssertionFailedError final : public std::exception {
    std::string fmt;

  public:
    explicit AssertionFailedError(
      StringView function,
      StringView source,
      StringView assertion_text,
      usize line,
      StringView msg
    );

    ~AssertionFailedError() override;

    const char *what() const noexcept override;
  };

  unit dbg_assert(
    StringView function,
    StringView source,
    StringView assertion_line,
    usize line,
    StringView msg
  );
}

#define debug_assert(condition, message) if (!static_cast<bool>(condition)) crab::debug::dbg_assert(\
  __FUNCTION__, \
  __FILE__, \
  #condition, \
  __LINE__, \
  (message))
#else

#define debug_assert(...)

#endif

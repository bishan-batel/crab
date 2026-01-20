#pragma once

#include "crab/core.hpp"
#include "crab/term/Handle.hpp"
#include "crab/core/unreachable.hpp"
#include "crab/num/integer.hpp"

#if CRAB_UNIX
#include <unistd.h>
#elif CRAB_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace crab::term::impl {
  CRAB_PURE CRAB_INLINE constexpr auto handle_to_descriptor(Handle handle) -> num::u32 {
#if CRAB_UNIX
    switch (handle) {
      case Handle::Out: return STDOUT_FILENO;
      case Handle::Error: return STDERR_FILENO;
      case Handle::Input: return STDIN_FILENO;
#else
    case Handle::Out: return STD_OUTPUT_HANDLE;
    case Handle::Error: return STD_ERROR_HANDLE;
    case Handle::Input: return STD_INPUT_HANDLE;
#endif
    }

    unreachable();
  }
}

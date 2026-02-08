///@file crab/term/impl/handle_to_descriptor.hpp

#pragma once

#include "crab/core.hpp"
#include "crab/term/Handle.hpp"
#include "crab/core/unreachable.hpp"
#include "crab/num/integer.hpp"

#if CRAB_UNIX
#include <unistd.h>
#elif CRAB_WIN32
#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#else
#include <windows.h>
#endif
#endif

namespace crab::term::impl {

  /// @internal
  /// Internal method to map a crab::term::Handle to its raw file handle
  ///
  /// @return Raw file handle
  CRAB_PURE CRAB_INLINE constexpr auto handle_to_descriptor(const Handle handle) -> u32 {
    switch (handle) {
#if CRAB_UNIX
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

#pragma once

#include "crab/term/forward.hpp"
#include "crab/term/impl/handle_to_descriptor.hpp"

namespace crab::term {

  /**
   * Attempts to enable ansii for the given handle, ex. enabling ansii when printing to stderr by using
   * try_enable_ansi(Handle::Error)
   */
  [[nodiscard]] inline static auto try_enable_ansi(Handle handle) -> bool {
    return try_enable_ansi_with_raw_handle(impl::handle_to_descriptor(handle));
  }

  [[nodiscard]] inline static auto try_enable_ansi_with_raw_handle(u32 handle) -> bool {
#if CRAB_UNIX
    return isatty(static_cast<int>(handle));
#else
    HANDLE handle_out{GetStdHandle(static_cast<DWORD>(handle))};

    if (handle_out == INVALID_HANDLE_VALUE) {
      return false;
    }

    DWORD mode = 0;

    if (not GetConsoleMode(hOut, &mode)) {
      return false;
    }

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    return SetConsoleMode(handle_out, mode);
#endif
  }

}

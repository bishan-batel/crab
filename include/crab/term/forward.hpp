
#include "crab/preamble.hpp"
#include "crab/term/Handle.hpp"

namespace crab::term {

  [[nodiscard]] inline static auto try_enable_ansi(Handle handle = Handle::Out) -> bool;

  [[nodiscard]] inline static auto try_enable_ansi_with_raw_handle(u32 handle) -> bool;
}

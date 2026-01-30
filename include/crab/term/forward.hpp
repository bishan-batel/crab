/// @file crab/term/forward.hpp

#pragma once

#include "crab/num/integer.hpp"
#include "crab/term/Handle.hpp"

namespace crab::term {

  [[nodiscard]] auto try_enable_ansi(Handle handle = Handle::Out) -> bool;

  [[nodiscard]] auto try_enable_ansi_with_raw_handle(num::u32 handle) -> bool;
}

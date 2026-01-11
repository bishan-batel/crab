#pragma once

#include "crab/preamble.hpp"

namespace crab::env {
  [[nodiscard]] auto check_flag(StringView name) -> bool;
}

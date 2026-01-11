#pragma once

#include "crab/str/str.hpp"

namespace crab::env {
  [[nodiscard]] auto check_flag(StringView name) -> bool;
}

#pragma once

#include <cstdlib>
#include "crab/opt/Option.hpp"
#include "crab/preamble.hpp"

namespace crab::env {

  /**
   * Attempts to pull an environment variable
   */
  [[nodiscard]] inline auto get_as_string(const StringView name) -> opt::Option<StringView> {
    const char* variable{std::getenv(name.data())};

    if (variable == nullptr) {
      return {};
    }

    return {StringView{variable}};
  }
}

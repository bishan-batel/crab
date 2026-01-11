#pragma once

#include <cstdlib>

#include "crab/opt/Option.hpp"
#include "crab/str/str.hpp"

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

  /**
   * Attempts to pull an environment variable
   */
  [[nodiscard]] inline auto check_flag(const StringView name) -> bool {
    return get_as_string(name).is_some_and([](StringView value) { return value == "1"; });
  }
}

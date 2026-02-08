/// @file crab/env/env.hpp
/// @ingroup env

#pragma once

#include <cstdlib>

#include "crab/opt/Option.hpp"
#include "crab/str/str.hpp"

/// @defgroup env Environment
/// Small helpers for manipulation of the external program environment variables.

/// @addtogroup env
/// @{

/// @namespace crab::env
/// This relatively small namespace includes functions relating to environment variables and manipulation of such.
namespace crab::env {

  /// Attempts to read an environment variable with the given name.
  ///
  /// @param name Verbatim name of the environment variable
  /// @returns The value of the environment variable if it exists, else none.
  [[nodiscard]] inline auto get_as_string(const StringView name) -> opt::Option<String> {
    const char* variable{std::getenv(name.data())};

    if (variable == nullptr) {
      return {};
    }

    return {String{variable}};
  }

  /// Helper for reading environment variables as boolean flags. If no environment variable
  /// with the given name was found OR the environment variable does not equal 1, this returns false.
  /// Else, this returns true.
  ///
  /// @brief Name of the environment variable
  /// @returns State of the flag
  [[nodiscard]] inline auto check_flag(const StringView name) -> bool {
    return get_as_string(name).is_some_and([](const String& value) { return value == "1"; });
  }
}

/// }@


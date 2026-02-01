/// @file crab/core/cases.hpp

#pragma once

namespace crab {

  /// @ingroup core
  /// Utility class for easily creating a Visitor instance when using
  /// std::visit and alike. The basis of this class is turning many functors into one for use with the 'Visitor'
  /// pattern.
  ///
  ///
  /// # Examples
  /// ```cpp
  /// std::variant<i32, String> v = String{"hello"};
  ///
  /// std::visit(
  ///   crab::cases{
  ///     [](const i32 &i) { fmt::println("Value is i32: {}", i); },
  ///     [](const String &s) { fmt::println("Value is string: {}", s); }
  ///   },
  ///   v
  /// );
  /// ```
  template<typename... Functions>
  struct cases final : Functions... {
    using Functions::operator()...;
  };

}

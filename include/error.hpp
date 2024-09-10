#pragma once
#include "preamble.hpp"

namespace crab {
  namespace error {
    class todo_exception final : public std::exception {
      [[nodiscard]] auto what() const noexcept -> const char * final { return "This function is not implemented yet."; }
    };
  }; // namespace error

  /**
   * Does not return, use when you are waiting to implement a function.
   */
  template<typename>
  [[noreturn]] unit todo() {
#if DEBUG
    throw error::todo_exception{};
#else
    static_assert(false, "Cannot compile on release with lingering TODOs");
#endif
  };
}; // namespace crab

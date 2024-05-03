#pragma once
#include "preamble.hpp"

namespace crab {
  namespace error {
    class todo_exception final : public std::exception {
      const char *what() const noexcept override;
    };
  };

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
};

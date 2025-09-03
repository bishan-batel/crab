#pragma once
#include <format>
#include "preamble.hpp"

namespace crab {
  namespace error {
    class todo_exception final : public std::exception {
      String msg;

    public:

      explicit todo_exception(const String& msg):
          msg{fmt::format("TODO Exception: {}", msg)} {}

      [[nodiscard]] auto what() const noexcept -> const char* final {
        return msg.c_str();
      }
    };
  }; // namespace error

  /**
   * Does not return, use when you are waiting to implement a function.
   */
  template<typename... T>
  [[noreturn]] unit todo(const String& msg, T&&...) {
#if DEBUG
    throw error::todo_exception{msg};
#else
    static_assert(false, "Cannot compile on release with lingering TODOs");
#endif
  };
}; // namespace crab

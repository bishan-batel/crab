#pragma once

#include "./preamble.hpp"
#include "./fmt.hpp"

namespace crab {
  namespace error {
    class todo_exception final : public std::exception {
      String msg;

    public:

      explicit todo_exception(const String& msg): msg{crab::format("TODO Exception: {}", msg)} {}

      CRAB_NODISCARD auto what() const noexcept -> const char* final {
        return msg.c_str();
      }
    };
  }; // namespace error

  /**
   * Does not return, use when you are waiting to implement a function.
   */
  template<typename... ArgsToIgnore>
  CRAB_NORETURN unit todo(const String& msg, ArgsToIgnore&&... args) {
    ((std::ignore = args), ...);

#if NDEBUG
    std::ignore = msg;
    static_assert(false, "Cannot compile on release with lingering TODOs");
#else
    throw error::todo_exception{msg};
#endif
  };
}; // namespace crab

#pragma once

#include "./preamble.hpp"
#include "./fmt.hpp"
#include "execinfo.h"
#include <iterator>
#include <stdexcept>

namespace crab {
  namespace helper {
    /**
     * Does not return, use when you are waiting to implement a function.
     */
    template<typename... ArgsToIgnore>
    CRAB_NORETURN unit todo(const String& message, ArgsToIgnore&&...) {
      throw std::runtime_error{message};
    };

  };

};

#define CRAB_TODO(...)                                                                                                 \
  CRAB_WARNING("Unimplemented Function")                                                                               \
  do {                                                                                                                 \
    ::crab::discard(__VA_ARGS__);                                                                                      \
    ::crab::helper::todo(crab::format("Function {} is unimplemented (TODO)", __func__));                               \
  } while (false)

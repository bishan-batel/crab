#pragma once

#include "crab/core/discard.hpp"
#include "crab/assertion/panic.hpp"
#include "crab/assertion/fmt.hpp"

/**
 * Does not return, use when you are waiting to implement a function.
 */
#define CRAB_TODO(...)                                                                                                 \
  CRAB_WARNING("Unimplemented Function")                                                                               \
  do {                                                                                                                 \
    ::crab::discard(__VA_ARGS__);                                                                                      \
    ::crab::assertion::panic(                                                                                          \
      crab::format("Function {} is unimplemented (TODO)", __func__),                                                   \
      ::crab::SourceLocation::current()                                                                                \
    );                                                                                                                 \
  } while (false)

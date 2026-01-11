#pragma once

#include "crab/fmt.hpp"
#include "crab/assertion/forward.hpp"

#if !NDEBUG

#define debug_assert_transparent(condition, source_location, ...)                                                      \
  if (not static_cast<bool>(condition)) [[unlikely]]                                                                   \
    do {                                                                                                               \
      ::crab::assertion::panic(                                                                                        \
        ::crab::format("{} (Assertion \"" #condition "\" Failed)", ::crab::format(__VA_ARGS__)),                       \
        source_location                                                                                                \
      );                                                                                                               \
  } while (false)

#else

#define debug_assert_transparent(condition, ...)                                                                       \
  do {                                                                                                                 \
    if (not static_cast<bool>(condition)) {                                                                            \
      crab::discard(__VA_ARGS__);                                                                                      \
      ::crab::unreachable();                                                                                           \
    }                                                                                                                  \
  } while (false)

#endif

#define debug_assert(condition, ...) debug_assert_transparent(condition, ::crab::SourceLocation::current(), __VA_ARGS__)

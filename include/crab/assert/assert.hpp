#pragma once

#include <cassert>

namespace crab::assert::impl {

}

#if !NDEBUG

#define crab_assert_transparent(condition, source_location, ...)                                                       \
  if (not static_cast<bool>(condition)) [[unlikely]]                                                                   \
    do {                                                                                                               \
      ::crab::debug::dbg_assert(source_location, #condition, ::crab::format(__VA_ARGS__));                             \
  } while (false)

#else

#define dbg_assert_transparent(condition, source_location, ...)                                                        \
  do {                                                                                                                 \
    if (not static_cast<bool>(condition)) {                                                                            \
      crab::discard(source_location, __VA_ARGS__);                                                                     \
      ::crab::unreachable();                                                                                           \
    }                                                                                                                  \
  } while (false)

#endif

#define debug_assert(condition, ...) debug_assert_transparent(condition, ::crab::SourceLocation::current(), __VA_ARGS__)

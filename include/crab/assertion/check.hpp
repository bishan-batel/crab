#pragma once

#include "crab/core/SourceLocation.hpp"
#include "crab/assertion/fmt.hpp"
#include "crab/assertion/panic.hpp"

#define crab_check_with_location(condition, source_location, ...)                                                      \
  if (not static_cast<bool>(condition)) [[unlikely]]                                                                   \
    do {                                                                                                               \
      ::crab::assertion::panic(                                                                                        \
        ::crab::format("{} (Check \"" #condition "\" Failed)", ::crab::format(__VA_ARGS__)),                           \
        source_location                                                                                                \
      );                                                                                                               \
  } while (false)

#define crab_check(condition, ...) crab_check_with_location(condition, ::crab::SourceLocation::current(), __VA_ARGS__)

#if CRAB_DEBUG

#define crab_dbg_check_with_location(condition, source_location, ...)                                                  \
  crab_check_with_location(condition, source_location, __VA_ARGS__)

#else

#include "crab/core/unreachable.hpp"
#include "crab/core/discard.hpp"

#define crab_dbg_check_with_location(condition, ...)                                                                   \
  do {                                                                                                                 \
    if (not static_cast<bool>(condition)) {                                                                            \
      ::crab::discard(__VA_ARGS__);                                                                                    \
      ::crab::unreachable();                                                                                           \
    }                                                                                                                  \
  } while (false)

#endif

#define crab_dbg_check(condition, ...)                                                                                 \
  crab_dbg_check_with_location(condition, ::crab::SourceLocation::current(), __VA_ARGS__)

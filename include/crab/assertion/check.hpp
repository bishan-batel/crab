#pragma once

#include "crab/core/SourceLocation.hpp"
#include "crab/core/discard.hpp"
#include "crab/assertion/fmt.hpp"
#include "crab/assertion/panic.hpp"

/// @addtogroup assertion
/// @{

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

/// Macro for conditionally calling crab_dbg_check vs crab_check based off of a constexpr bool. If dbg_only is true,
/// then this will perform a crab_dbg_check, else this will expand to crab_check
#define crab_cond_check(dbg_only, cond, ...)                                                                           \
  if constexpr (dbg_only) {                                                                                            \
    crab_dbg_check(cond, __VA_ARGS__);                                                                                 \
    ::crab::discard(cond); /* required to supress warnings of identical branches */                                                                                        \
  } else {                                                                                                             \
    crab_check(cond, __VA_ARGS__);                                                                                     \
  }

/// }@

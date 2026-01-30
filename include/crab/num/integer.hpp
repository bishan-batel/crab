/// @file crab/num/integer.hpp
/// @ingroup num

#pragma once

#include <cstddef>
#include <cstdint>

#include "crab/core.hpp"

/// @addtogroup num
/// @{

  namespace crab::num {
  /// Fix Sized Unsigned 8 Bit Integer (cannot be negative)
  using u8 = std::uint8_t;

  /// Fix Sized Unsigned 16 Bit Integer (cannot be negative)
  using u16 = std::uint16_t;

  /// Fix Sized Unsigned 32 Bit Integer (cannot be negative)
  using u32 = std::uint32_t;

  /// Fix Sized Unsigned 64 Bit Integer (cannot be negative)
  using u64 = std::uint64_t;

  /// Biggest Unsigned Integer type that the current platform can use
  /// (cannot be negative)
  using umax = std::uintmax_t;

  /// Unsigned Integer for when referring to any form of memory size or
  /// offset (eg. an array length or index)
  using usize = std::size_t;

  /// Signed memory offset
  using ptrdiff = std::ptrdiff_t;

  /// Unsigned Integer Pointer typically used for pointer arithmetic
  using uptr = std::uintptr_t;

  /// Signed 8 bit Integer
  using i8 = std::int8_t;

  /// Signed 16 bit Integer
  using i16 = std::int16_t;

  /// Signed 32 bit Integer
  using i32 = std::int32_t;

  /// Signed 64 bit Integer
  using i64 = std::int64_t;

  /// Biggest Integer type that the current platform can use
  using imax = std::intmax_t;

  /// Integer pointer typically used for pointer arithmetic
  using iptr = std::intptr_t;

#if __cpp_char8_t
  /// @brief UTF-8 Encoded Character
  using char8 = char8_t;
#else
  /// UTF-8 Encoded Character
  using char8 = char;
#endif

#if __cpp_unicode_characters
  /// UTF-16 Encoded Character
  using char16 = char16_t;

  /// UTF-32 Encoded Character
  using char32 = char32_t;
#else
  /// UTF-16 Encoded Character
  using char16 = u16;

  /// UTF-32 Encoded Character
  using char32 = u32;
#endif
}

/// }@

namespace crab {
  using namespace crab::num;

  namespace prelude {
    using namespace num;
  }
}

CRAB_PRELUDE_GUARD;

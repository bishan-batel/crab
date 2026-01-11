#pragma once

#include <cstddef>
#include <cstdint>

#include "crab/core.hpp"

namespace crab::num {
  /**
   * @brief Fix Sized Unsigned 8 Bit Integer (cannot be negative)
   */
  using u8 = std::uint8_t;

  /**
   * @brief Fix Sized Unsigned 16 Bit Integer (cannot be negative)
   */
  using u16 = std::uint16_t;

  /**
   * @brief Fix Sized Unsigned 32 Bit Integer (cannot be negative)
   */
  using u32 = std::uint32_t;

  /**
   * @brief Fix Sized Unsigned 64 Bit Integer (cannot be negative)
   */
  using u64 = std::uint64_t;
  /**
   * @brief Biggest Unsigned Integer type that the current platform can use
   * (cannot be negative)
   */
  using umax = std::uintmax_t;

  /**
   * @brief Unsigned Integer for when referring to any form of memory size or
   * offset (eg. an array length or index)
   */
  using usize = std::size_t;

  /**
   * @brief Signed memory offset
   */
  using ptrdiff = std::ptrdiff_t;
  /**
   * @brief Unsigned Integer Pointer typically used for pointer arithmetic
   */
  using uptr = std::uintptr_t;

  /**
   * @brief Signed 8 bit Integer
   */
  using i8 = std::int8_t;

  /**
   * @brief Signed 16 bit Integer
   */
  using i16 = std::int16_t;

  /**
   * @brief Signed 32 bit Integer
   */
  using i32 = std::int32_t;

  /**
   * @brief Signed 64 bit Integer
   */
  using i64 = std::int64_t;

  /**
   * @brief Biggest Integer type that the current platform can use
   */
  using imax = std::intmax_t;

  /**
   * @brief Integer pointer typically used for pointer arithmetic
   */
  using iptr = std::intptr_t;

#if __cpp_char8_t
  /**
   * @brief UTF-8 Encoded Character
   */
  using char8 = char8_t;
#else
  /**
   * @brief UTF-8 Encoded Character
   */
  using char8 = char;
#endif

#if __cpp_unicode_characters
  /**
   * @brief UTF-16 Encoded Character
   */
  using char16 = char16_t;

  /**
   * @brief UTF-32 Encoded Character
   */
  using char32 = char32_t;
#else
  /**
   * @brief UTF-16 Encoded Character
   */
  using char16 = u16;

  /**
   * @brief UTF-32 Encoded Character
   */
  using char32 = u32;
#endif
}

namespace crab {
  using namespace crab::num;

  namespace prelude {
    using namespace num;
  }
}

CRAB_PRELUDE_GUARD;

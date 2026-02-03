/// @file crab/num/suffixes.hpp
/// @ingroup num

#pragma once

#include <numbers>
#include "crab/core.hpp"
#include "crab/num/floating.hpp"
#include "crab/num/integer.hpp"

namespace crab::num::suffixes {
  /// @addtogroup prelude
  /// @{
  /// @addtogroup num
  /// @{

  /// Literal for converting a degree literal -> radians
  CRAB_PURE CRAB_INLINE constexpr f32 operator""_deg(const f64 literal) {
    return static_cast<f32>(literal * std::numbers::pi / 180.f);
  }

  /// Converts literal to an f32
  CRAB_PURE CRAB_INLINE constexpr f32 operator""_f32(const f64 literal) {
    return static_cast<f32>(literal);
  }

  /// Converts literal to an f64
  CRAB_PURE CRAB_INLINE constexpr f64 operator""_f64(const f64 literal) {
    return literal;
  }

  /// Converts literal to an i8
  CRAB_PURE CRAB_INLINE constexpr i8 operator""_i8(const unsigned long long literal) {
    return static_cast<i8>(literal);
  }

  /// Converts literal to an i16
  CRAB_PURE CRAB_INLINE constexpr i16 operator""_i16(const unsigned long long literal) {
    return static_cast<i16>(literal);
  }

  /// Converts literal to an i32
  CRAB_PURE CRAB_INLINE constexpr i32 operator""_i32(const unsigned long long literal) {
    return static_cast<i32>(literal);
  }

  /// Converts literal to an i64
  CRAB_PURE CRAB_INLINE constexpr i64 operator""_i64(const unsigned long long literal) {
    return static_cast<i64>(literal);
  }

  /// Converts literal to an imax
  CRAB_PURE CRAB_INLINE constexpr imax operator""_imax(const unsigned long long literal) {
    return static_cast<imax>(literal);
  }

  /// Converts literal to an iptr
  CRAB_PURE CRAB_INLINE constexpr iptr operator""_iptr(const unsigned long long literal) {
    return static_cast<iptr>(literal);
  }

  /// Converts literal to an u8
  CRAB_PURE CRAB_INLINE constexpr u8 operator""_u8(const unsigned long long literal) {
    return static_cast<u8>(literal);
  }

  /// Converts literal to an u16
  CRAB_PURE CRAB_INLINE constexpr u16 operator""_u16(const unsigned long long literal) {
    return static_cast<u16>(literal);
  }

  /// Converts literal to an u32
  CRAB_PURE CRAB_INLINE constexpr u32 operator""_u32(const unsigned long long literal) {
    return static_cast<u32>(literal);
  }

  /// Converts literal to an u64
  CRAB_PURE CRAB_INLINE constexpr u64 operator""_u64(const unsigned long long literal) {
    return static_cast<u64>(literal);
  }

  /// Converts literal to an usize
  CRAB_PURE CRAB_INLINE constexpr usize operator""_usize(const unsigned long long literal) {
    return static_cast<usize>(literal);
  }

  /// Converts literal to an umax
  CRAB_PURE CRAB_INLINE constexpr umax operator""_umax(const unsigned long long literal) {
    return static_cast<umax>(literal);
  }

  /// Converts literal to an uptr
  CRAB_PURE CRAB_INLINE constexpr uptr operator""_uptr(const unsigned long long literal) {
    return static_cast<uptr>(literal);
  }

  /// }@
  /// }@
}

namespace crab::prelude {
  using namespace crab::num::suffixes;
}

CRAB_PRELUDE_GUARD;

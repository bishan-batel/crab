#pragma once

#include <numbers>
#include "crab/core.hpp"
#include "crab/num/floating.hpp"
#include "crab/num/integer.hpp"

namespace crab::num::suffixes {
  /**
   * @brief Literal for converting a degree literal -> radians
   */
  [[nodiscard]] CRAB_INLINE constexpr f32 operator""_deg(const f64 literal) {
    return static_cast<f32>(literal * std::numbers::pi / 180.f);
  }

  [[nodiscard]] CRAB_INLINE constexpr f32 operator""_f32(const f64 literal) {
    return static_cast<f32>(literal);
  }

  [[nodiscard]] CRAB_INLINE constexpr f64 operator""_f64(const f64 literal) {
    return literal;
  }

  /**
   * @brief Converts literal to an i8
   */
  [[nodiscard]] CRAB_INLINE constexpr i8 operator""_i8(const unsigned long long literal) {
    return static_cast<i8>(literal);
  }

  /**
   * @brief Converts literal to an i16
   */
  [[nodiscard]] CRAB_INLINE constexpr i16 operator""_i16(const unsigned long long literal) {
    return static_cast<i16>(literal);
  }

  /**
   * @brief Converts literal to an i32
   */
  [[nodiscard]] CRAB_INLINE constexpr i32 operator""_i32(const unsigned long long literal) {
    return static_cast<i32>(literal);
  }

  /**
   * @brief Converts literal to an i64
   */
  [[nodiscard]] CRAB_INLINE constexpr i64 operator""_i64(const unsigned long long literal) {
    return static_cast<i64>(literal);
  }

  /**
   * @brief Converts literal to an imax
   */
  [[nodiscard]] CRAB_INLINE constexpr imax operator""_imax(const unsigned long long literal) {
    return static_cast<imax>(literal);
  }

  /**
   * @brief Converts literal to an iptr
   */
  [[nodiscard]] CRAB_INLINE constexpr iptr operator""_iptr(const unsigned long long literal) {
    return static_cast<iptr>(literal);
  }

  /**
   * @brief Converts literal to an u8
   */
  [[nodiscard]] CRAB_INLINE constexpr u8 operator""_u8(const unsigned long long literal) {
    return static_cast<u8>(literal);
  }

  /**
   * @brief Converts literal to an u16
   */
  [[nodiscard]] CRAB_INLINE constexpr u16 operator""_u16(const unsigned long long literal) {
    return static_cast<u16>(literal);
  }

  /**
   * @brief Converts literal to an u32
   */
  [[nodiscard]] CRAB_INLINE constexpr u32 operator""_u32(const unsigned long long literal) {
    return static_cast<u32>(literal);
  }

  /**
   * @brief Converts literal to an u64
   */
  [[nodiscard]] CRAB_INLINE constexpr u64 operator""_u64(const unsigned long long literal) {
    return static_cast<u64>(literal);
  }

  /**
   * @brief Converts literal to an usize
   */
  [[nodiscard]] CRAB_INLINE constexpr usize operator""_usize(const unsigned long long literal) {
    return static_cast<usize>(literal);
  }

  /**
   * @brief Converts literal to an umax
   */
  [[nodiscard]] CRAB_INLINE constexpr umax operator""_umax(const unsigned long long literal) {
    return static_cast<umax>(literal);
  }

  /**
   * @brief Converts literal to an uptr
   */
  [[nodiscard]] CRAB_INLINE constexpr uptr operator""_uptr(const unsigned long long literal) {
    return static_cast<uptr>(literal);
  }
}

namespace crab::prelude {
  using namespace crab::num::suffixes;
}

CRAB_PRELUDE_GUARD;

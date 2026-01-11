#pragma once

#include <numbers>
#include "crab/core.hpp"
#include "crab/num/floating.hpp"
#include "crab/num/integer.hpp"

namespace crab::num::suffixes {
  /**
   * @brief Literal for converting a degree literal -> radians
   */
  CRAB_NODISCARD_CONSTEVAL f32 operator""_deg(const f64 literal) {
    return static_cast<f32>(literal * std::numbers::pi / 180.f);
  }

  CRAB_NODISCARD_CONSTEVAL f32 operator""_f32(const f64 literal) {
    return static_cast<f32>(literal);
  }

  CRAB_NODISCARD_CONSTEVAL f64 operator""_f64(const f64 literal) {
    return literal;
  }

  /**
   * @brief Converts literal to an i8
   */
  CRAB_NODISCARD_CONSTEVAL i8 operator""_i8(const unsigned long long literal) {
    return static_cast<i8>(literal);
  }

  /**
   * @brief Converts literal to an i16
   */
  CRAB_NODISCARD_CONSTEVAL i16 operator""_i16(const unsigned long long literal) {
    return static_cast<i16>(literal);
  }

  /**
   * @brief Converts literal to an i32
   */
  CRAB_NODISCARD_CONSTEVAL i32 operator""_i32(const unsigned long long literal) {
    return static_cast<i32>(literal);
  }

  /**
   * @brief Converts literal to an i64
   */
  CRAB_NODISCARD_CONSTEVAL i64 operator""_i64(const unsigned long long literal) {
    return static_cast<i64>(literal);
  }

  /**
   * @brief Converts literal to an imax
   */
  CRAB_NODISCARD_CONSTEVAL imax operator""_imax(const unsigned long long literal) {
    return static_cast<imax>(literal);
  }

  /**
   * @brief Converts literal to an iptr
   */
  CRAB_NODISCARD_CONSTEVAL iptr operator""_iptr(const unsigned long long literal) {
    return static_cast<iptr>(literal);
  }

  /**
   * @brief Converts literal to an u8
   */
  CRAB_NODISCARD_CONSTEVAL u8 operator""_u8(const unsigned long long literal) {
    return static_cast<u8>(literal);
  }

  /**
   * @brief Converts literal to an u16
   */
  CRAB_NODISCARD_CONSTEVAL u16 operator""_u16(const unsigned long long literal) {
    return static_cast<u16>(literal);
  }

  /**
   * @brief Converts literal to an u32
   */
  CRAB_NODISCARD_CONSTEVAL u32 operator""_u32(const unsigned long long literal) {
    return static_cast<u32>(literal);
  }

  /**
   * @brief Converts literal to an u64
   */
  CRAB_NODISCARD_CONSTEVAL u64 operator""_u64(const unsigned long long literal) {
    return static_cast<u64>(literal);
  }

  /**
   * @brief Converts literal to an usize
   */
  CRAB_NODISCARD_CONSTEVAL usize operator""_usize(const unsigned long long literal) {
    return static_cast<usize>(literal);
  }

  /**
   * @brief Converts literal to an umax
   */
  CRAB_NODISCARD_CONSTEVAL umax operator""_umax(const unsigned long long literal) {
    return static_cast<umax>(literal);
  }

  /**
   * @brief Converts literal to an uptr
   */
  CRAB_NODISCARD_CONSTEVAL uptr operator""_uptr(const unsigned long long literal) {
    return static_cast<uptr>(literal);
  }
}

namespace crab::prelude {
  using namespace crab::num::suffixes;
}

CRAB_PRELUDE_GUARD;

/**
 * Created by Kishan Patel (bishan.batel@protonmail.com, kishan.patel@digipen.edu) on 3/3/2024
 */

// ReSharper disable CppUnusedIncludeDirective
#pragma once

#include <cstdint>
#include <numbers>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>

/**
 * \brief 32 Bit Floating Point Number
 */
using f32 = float;

/**
 * \brief 64 Bit Floating Point Number
 */
using f64 = long double;

/**
 * \brief Unsigned 8 Bit Integer (cannot be negative)
 */
using u8 = std::uint8_t;

/**
 * \brief Unsigned 16 Bit Integer (cannot be negative)
 */
using u16 = std::uint16_t;

/**
 * \brief Unsigned 32 Bit Integer (cannot be negative)
 */
using u32 = std::uint32_t;

/**
 * \brief Unsigned 64 Bit Integer (cannot be negative)
 */
using u64 = std::uint64_t;

/**
 * \brief Biggest Unsigned Integer type that the current platform can use
 * (cannot be negative)
 */
using umax = std::uintmax_t;

/**
 * \brief Unsigned Integer for when referring to any form of memory size or
 * offset (eg. an array length or index)
 */
using usize = std::size_t;

/**
 * \brief Unsigned Integer Pointer typically used for pointer arithmetic
 */
using uptr = std::uintptr_t;

/**
 * \brief Signed 8 bit Integer
 */
using i8 = std::int8_t;

/**
 * \brief Signed 16 bit Integer
 */
using i16 = std::int16_t;

/**
 * \brief Signed 32 bit Integer
 */
using i32 = std::int32_t;

/**
 * \brief Signed 64 bit Integer
 */
using i64 = std::int64_t;

/**
 * \brief Biggest Integer type that the current platform can use
 */
using imax = std::intmax_t;

/**
 * \brief Integer pointer typically used for pointer arithmetic
 */
using iptr = std::intptr_t;

/**
 * \brief UTF-8 Encoded Character
 */
using u8char = char8_t;

/**
 * \brief UTF-16 Encoded Character
 */
using u16char = char16_t;

/**
 * \brief UTF-32 Encoded Character
 */
using u32char = char32_t;

/**
 * \brief std::string, fat pointer to a heap allocated string
 */
using String = std::string;

/**
 * Abstraction over any contiguous sequence of characters, always prefer this
 * over const String&
 */
using StringView = std::string_view;

/**
 * Abstraction over any contiguous sequence of elements
 */
template<typename T, usize length = std::dynamic_extent>
using Span = std::span<T, length>;

/**
 * Heap allocated, dynamically sized list
 */
template<typename T>
using Vec = std::vector<T>;

/**
 * Unordered set of elements
 */
template<typename T, typename Hash = std::hash<T>,
         typename Predicate = std::equal_to<T>>
using Set = std::unordered_set<T, Hash, Predicate>;

/**
 * Unordered key-value collection
 */
template<typename Key, typename Value, typename Hash = std::hash<Key>,
         typename Predicate = std::equal_to<Key>>
using Dictionary = std::unordered_map<Key, Value, Hash, Predicate>;

/**
 * \brief 0 Sized Type
 */
struct unit {
  [[nodiscard]] bool operator==(const unit &) const { return true; }
};

constexpr f32 operator""_deg(const f64 literal) {
  return static_cast<f32>(literal * std::numbers::pi / 180.f);
}

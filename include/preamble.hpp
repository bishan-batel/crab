/**
 * Created by Kishan Patel (bishan.batel@protonmail.com, kishan.patel@digipen.edu) on 3/3/2024
 */

// ReSharper disable CppUnusedIncludeDirective
#pragma once

#include <cstdint>
#include <functional>
#include <numbers>
#include <ostream>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define nameof(x) #x

template<typename T>
using Raw = T *;

/**
 * \brief 32 Bit Floating Point Number
 */
using f32 = float;

/**
 * \brief 64 Bit Floating Point Number
 */
using f64 = long double;

/**
 * \brief Fix Sized Unsigned 8 Bit Integer (cannot be negative)
 */
using u8 = std::uint8_t;

/**
 * \brief Fix Sized Unsigned 16 Bit Integer (cannot be negative)
 */
using u16 = std::uint16_t;

/**
 * \brief Fix Sized Unsigned 32 Bit Integer (cannot be negative)
 */
using u32 = std::uint32_t;

/**
 * \brief Fix Sized Unsigned 64 Bit Integer (cannot be negative)
 */
using u64 = unsigned long int;

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
using char8 = char8_t;

/**
 * \brief UTF-16 Encoded Character
 */
using char16 = char16_t;

/**
 * \brief UTF-32 Encoded Character
 */
using char32 = char32_t;

/**
 * \brief std::string, fat pointer to a heap allocated string
 */
using String = std::string;

/**
 * \brief UTF Encoded Character
 */
using widechar = wchar_t;

/**
 * \brief std::wstring, fat pointer to a heap allocated unicode string
 */
using WideString = std::wstring;

/**
 * \brief Abstraction over any contiguous sequence of characters, always prefer this
 * over const String&
 */
using StringView = std::string_view;

/**
 * \brief Abstraction over any contiguous sequence of unicode characters, always prefer this
 * over const WideString&
 */
using WideStringView = std::wstring_view;

/**
 * \brief std::stringstream
 */
using StringStream = std::stringstream;

/**
 * \brief std::stringstream
 */
using OutStringStream = std::ostringstream;

/**
 * \brief std::stringstream
 */
using InStringStream = std::istringstream;

/**
 * \brief std::wstringstream
 */
using WideStringStream = std::wstringstream;

/**
 * @brief Function pointer that supports lambdas with captures
 */
template<typename F = void()>
using Func = std::function<F>;

/**
 * \brief std::tuple<T...> alias.
 */
template<typename... Types>
using Tuple = std::tuple<Types...>;

/**
 * \brief std::pair<T, S> alias.
 */
template<typename A, typename B>
using Pair = std::pair<A, B>;

/**
 * std::ranges
 */
namespace ranges = std::ranges;

/**
 * std::ranges::views
 */
namespace views = std::ranges::views;

/**
 * @brief Alias for std::array
 *
 * Statically Sized list of packed objects
 *
 * @tparam T
 */
template<typename T, usize length>
using SizedArray = std::array<T, length>;

/**
 * \brief Abstraction over any contiguous sequence of elements
 */
template<typename T, usize length = std::dynamic_extent>
using Span = std::span<T, length>;

/**
 * \brief Heap allocated, dynamically sized list
 */
template<typename T>
using Vec = std::vector<T>;

/**
 * \brief Unordered set of elements
 */
template<typename T, typename Hash = std::hash<T>, typename Predicate = std::equal_to<T>>
using Set = std::unordered_set<T, Hash, Predicate>;

/**
 * \brief Unordered key-value collection
 */
template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Predicate = std::equal_to<Key>>
using Dictionary = std::unordered_map<Key, Value, Hash, Predicate>;

/**
 * \brief 0 Sized Type
 */
struct unit {
  static const unit val;

  constexpr unit() = default;

  [[nodiscard]] constexpr auto operator==(const unit &) const -> bool { return true; }
};

constexpr unit unit::val{};

inline auto operator<<(std::ostream &os, const unit &) -> std::ostream & { return os << "unit"; }

/**
 * \brief Literal for converting a degree literal -> radians
 */
constexpr f32 operator""_deg(const f64 literal) { return static_cast<f32>(literal * std::numbers::pi / 180.f); }

constexpr f32 operator""_f32(const f64 literal) { return static_cast<f32>(literal); }

constexpr f64 operator""_f64(const f64 literal) { return literal; }

#define crab_impl_literal(type)                                                                                        \
  constexpr type operator""_##type(const unsigned long long literal) { return static_cast<type>(literal); } // NOLINT

/**
 * \brief Converts literal to an i16
 */
crab_impl_literal(i16);

/**
 * \brief Converts literal to an i32
 */
crab_impl_literal(i32);

/**
 * \brief Converts literal to an i64
 */
crab_impl_literal(i64);

/**
 * \brief Converts literal to an imax
 */
crab_impl_literal(imax);

/**
 * \brief Converts literal to an iptr
 */
crab_impl_literal(iptr);

/**
 * \brief Converts literal to an u16
 */
crab_impl_literal(u16);

/**
 * \brief Converts literal to an u32
 */
crab_impl_literal(u32);

/**
 * \brief Converts literal to an u64
 */
crab_impl_literal(u64);

/**
 * \brief Converts literal to an umax
 */
crab_impl_literal(umax);

/**
 * \brief Converts literal to an uptr
 */
crab_impl_literal(uptr);

#undef crab_impl_literal

// Pattern Matching
namespace crab {
  /**
   * @brief Shows its use with std::visit
   */
  template<typename... Cases>
  struct cases : Cases... {
    using Cases::operator()...;
  };
} // namespace crab

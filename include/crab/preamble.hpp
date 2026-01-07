/**
 * Created by Kishan Patel (bishan.batel@protonmail.com) on 3/3/2024
 */

#pragma once

#include "./core.hpp"

#include <cstdint>
#include <functional>
#include <numbers>
#include <ostream>
#include <source_location>
#include <sstream>
#include <string>
#include <utility>
#include <array>
#include <span>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <ranges>

namespace crab {

  namespace num {
    /**
     * @brief 32 Bit Floating Point Number
     */
    using f32 = float;

    /**
     * @brief 64 Bit Floating Point Number
     */
    using f64 = long double;

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

  using namespace crab::num;

  /**
   * @brief std::string, fat pointer to a heap allocated string
   */
  using String = std::string;

  /**
   * @brief UTF Encoded Character
   */
  using widechar = wchar_t;

  /**
   * @brief std::wstring, fat pointer to a heap allocated unicode string
   */
  using WideString = std::wstring;

  /**
   * @brief Abstraction over any contiguous sequence of characters, always prefer
   * this over const String&
   */
  using StringView = std::string_view;

  /**
   * @brief Abstraction over any contiguous sequence of unicode characters, always
   * prefer this over const WideString&
   */
  using WideStringView = std::wstring_view;

  /**
   * @brief std::stringstream
   */
  using StringStream = std::stringstream;

  /**
   * @brief std::stringstream
   */
  using OutStringStream = std::ostringstream;

  /**
   * @brief std::stringstream
   */
  using InStringStream = std::istringstream;

  /**
   * @brief std::wstringstream
   */
  using WideStringStream = std::wstringstream;

  using SourceLocation = std::source_location;

  /**
   * @brief Function pointer that supports lambdas with captures
   */
  template<typename F = void()>
  using Func = std::function<F>;

  /**
   * @brief std::tuple<T...> alias.
   */
  template<typename... Types>
  using Tuple = std::tuple<Types...>;

  /**
   * @brief std::pair<T, S> alias.
   */
  template<typename A, typename B>
  using Pair = std::pair<A, B>;

  /**
   * @brief Monostate type, all instances of 'unit' are indistinguishable,
   * note that this type will never be '0' sized, unless being used as a field
   * with [[no_unique_address]]
   */
  struct unit {
    /**
     * Instance of unit, equivalent to constructing a new one, more of a stylistic
     * choice whether you want to type unit{} or unit::val, I personally use
     * unit::val to imply that there is a single value of unit
     */
    static const unit val;

    constexpr unit() = default;

    /**
     * Unit has not state, all instances of until are equal
     * (this will always return true)
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator==(const unit&) const -> bool {
      return true;
    }
  };

  inline const unit unit::val{};

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
   * @brief Abstraction over any contiguous sequence of elements
   */
  template<typename T, usize length = std::dynamic_extent>
  using Span = std::span<T, length>;

  /**
   * @brief Heap allocated, dynamically sized list
   */
  template<typename T>
  using Vec = std::vector<T>;

  /**
   * @brief Unordered set of elements
   */
  template<typename T, typename Hash = std::hash<T>, typename Predicate = std::equal_to<T>>
  using Set = std::unordered_set<T, Hash, Predicate>;

  /**
   * @brief Unordered key-value collection
   */
  template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Predicate = std::equal_to<Key>>
  using Dictionary = std::unordered_map<Key, Value, Hash, Predicate>;

  namespace numeric_suffixes {
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

  /**
   * @brief Utility class for easily creating a Visitor instance when using
   * std::visit and alike
   */
  template<typename... Functions>
  struct cases final : Functions... {
    using Functions::operator()...;
  };
} // namespace crab

inline auto operator<<(std::ostream& os, ::crab::unit) -> decltype(auto) {
  return os << "unit";
}

template<std::derived_from<std::ostream> T>
inline auto operator<<(T&& os, ::crab::unit) -> T&& {
  os << ::crab::unit::val;
  return os;
}

#if CRAB_USE_PRELUDE

using ::crab::widechar;
using ::crab::String;
using ::crab::WideString;
using ::crab::StringView;
using ::crab::StringStream;
using ::crab::OutStringStream;
using ::crab::InStringStream;
using ::crab::WideStringStream;
using ::crab::Func;
using ::crab::Tuple;
using ::crab::Pair;

#if !CRAB_NO_TYPEDEF_ARRAY
using ::crab::SizedArray;
#endif

#if !CRAB_NO_TYPEDEF_SPAN
using ::crab::Span;
#endif

#if !CRAB_NO_TYPEDEF_VEC
using ::crab::Vec;
#endif

#if !CRAB_NO_TYPEDEF_SET
using ::crab::Set;
#endif

#if !CRAB_NO_TYPEDEF_DICTIONARY
using ::crab::Dictionary;
#endif

using namespace ::crab::numeric_suffixes;
using namespace ::crab::num;

/**
 * std::ranges
 */
namespace ranges = ::std::ranges;

/**
 * std::ranges::views
 */
namespace views = ::std::ranges::views;

using ::crab::unit;
#endif

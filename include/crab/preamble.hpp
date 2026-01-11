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

#include "./num/integer.hpp"
#include "./num/floating.hpp"
#include "./num/suffixes.hpp"

namespace crab {
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

  /**
   * @brief Utility class for easily creating a Visitor instance when using
   * std::visit and alike
   */
  template<typename... Functions>
  struct cases final : Functions... {
    using Functions::operator()...;
  };

}

namespace crab::assertion {
  CRAB_NORETURN auto panic(StringView msg, SourceLocation loc) -> void;
}

namespace crab::prelude {

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

  using namespace ::crab::num::suffixes;
  using namespace ::crab::num;

  /**
   * std::ranges
   */
  namespace ranges = ::std::ranges;

  /**
   * std::ranges::views
   */
  namespace views = ::std::ranges::views;
}

CRAB_PRELUDE_GUARD;

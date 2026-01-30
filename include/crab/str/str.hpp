/// @file crab/str/str.hpp

#pragma once

#include <string>
#include "crab/core.hpp"

namespace crab {

  /// std::string alias. Heap allocated, owned string
  using String = std::string;

  /// UTF Encoded Character
  using widechar = wchar_t;

  /// std::wstring alias. Heap allocated, wide string.
  using WideString = std::wstring;

  /// Span over sequence of characters. You should almost always prefer this over passing values by
  /// const String / reference.
  using StringView = std::string_view;

  /// Span over sequence of unicode characters. You should almost always prefer this over
  /// passing values by / const String / reference.
  using WideStringView = std::wstring_view;

  /// Alias to std::stringstream
  using StringStream = std::stringstream;

  /// Alias to std::ostringstream
  using OutStringStream = std::ostringstream;

  /// Alias to std::istringstream
  using InStringStream = std::istringstream;

  /// Alias to std::wstringstream
  using WideStringStream = std::wstringstream;

}

namespace crab::prelude {
  using crab::String;
  using crab::widechar;
  using crab::WideString;
  using crab::StringView;
  using crab::WideStringView;
  using crab::StringStream;
  using crab::OutStringStream;
  using crab::InStringStream;
  using crab::WideStringStream;
}

CRAB_PRELUDE_GUARD;

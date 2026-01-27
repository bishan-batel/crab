#pragma once

#include <string>
#include "crab/core.hpp"

namespace crab {

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

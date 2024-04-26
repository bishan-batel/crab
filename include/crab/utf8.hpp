//
// Created by bishan_ on 4/26/24.
//

#pragma once

#include "../preamble.hpp"

namespace crab::utf8 {
  /**
   * \brief UTF-8 Encoded String
   */
  using String = std::basic_string<u8char>;

  /**
   * \brief UTF-8 Encoded String
   */
  using StringView = std::basic_string_view<u8char>;

  /**
   * \brief UTF-8 String Stream
   */
  using Stream = std::stringstream<u8char>;
}

//
// Created by bishan_ on 4/25/24.
//

#pragma once
#include <type_traits>

namespace crab {
  namespace ref {
    template<typename T>
    concept is_valid_type = not std::is_const_v<T> and not std::is_reference_v<T> and not std::is_volatile_v<T>;
  }

  template<typename T>
  concept is_complete_type = requires { sizeof(T); };
} // namespace crab

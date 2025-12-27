#pragma once

#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <crab/preamble.hpp>

namespace assert {

  template<typename... T>
  struct typelist_t {};

  template<typename... T>
  constexpr typelist_t<T...> types;

  template<typename T>
  struct type {};

  template<typename... Types>
  void for_types(const typelist_t<Types...>&, auto func) {
    std::ignore = std::make_tuple([&]() {
      func(type<Types>{});
      return unit{};
    }()...);
  }

  template<typename... Types>
  void for_types(auto func) {
    for_types(types<Types...>, func);
  }

};

#pragma once

#include <catch2/catch_test_macros.hpp>
#include <concepts>
#include <crab/preamble.hpp>
#include <utility>
#include "crab/type_traits.hpp"

namespace asserts {

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
      return crab::unit{};
    }()...);
  }

  template<typename... Types>
  void for_types(auto func) {
    for_types(types<Types...>, func);
  }

  namespace impl {
    template<typename T>
    struct dependent_false : std::false_type {};

    template<typename V, template<typename...> typename T, typename... Args>
    struct apply_template final : std::true_type {
      static_assert(dependent_false<T<Args...>>{});
    };

    template<template<typename...> typename T, typename... Args>
    struct apply_template<void, T, Args...> final : std::true_type {
      std::tuple<T<Args...>> t;
    };

    template<template<typename...> typename T, typename... Args>
    auto sfinae_helper(type<T<Args...>>) -> std::true_type {
      T<Args...>& a = *reinterpret_cast<T<Args...>*>(nullptr);
    }

    template<template<typename...> typename T, typename... Args>
    auto sfinae_helper(type<T<Args...>>) -> std::false_type {}
  }

  template<template<typename...> typename T, typename... Args>
  concept is_instantiation_valid = crab::complete_type<T<Args...>>;

};

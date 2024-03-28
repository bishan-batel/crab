//
// Created by bishan_ on 3/12/24.
//

// ReSharper disable  CppNonExplicitConvertingConstructor

// ReSharper disable CppDFAUnreachableCode
#pragma once
#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>

#include "preamble.hpp"
#include <iostream>

using None = unit;

template <typename T>
  requires(not std::is_const_v<T> and not std::is_reference_v<T>)
class Ref;

template <typename T>
  requires(not std::is_const_v<T> and not std::is_reference_v<T>)
class RefMut;

/**
 * Represents a value that could be None, this is almost
 * always a better alternative to using nullptrs.
 *
 * Tagged union type between T and unit
 */
template <typename T> class Option final {
public:
  Option(T &&from) noexcept : value(from) {}

  Option(None) noexcept : Option() {}

  Option() noexcept : value(None{}) {}

  Option &operator=(T &&from) {
    value = from;
    return *this;
  }

  Option &operator=(None) {
    value = None{};
    return *this;
  }

  /**
   * Whether this option has a contained value or not (None)
   */
  // ReSharper disable once CppNonExplicitConversionOperator
  [[nodiscard]] operator bool() const { return is_some(); }

  /**
   * Whether this option contains a value
   */
  [[nodiscard]] bool is_some() const {
    return std::holds_alternative<T>(value);
  }
  /**
   * Whether this option does not contain a value
   */
  [[nodiscard]] bool is_none() const {
    return std::holds_alternative<None>(value);
  }

  /**
   *
   */
  [[nodiscard]] auto map(const auto &transformer) {
    using M = decltype(transformer(take_unchecked()));

    static_assert(not std::is_rvalue_reference_v<M>,
                  "Cannot map to a Option<RValue Reference>");

    if (is_some()) {
      return Option<M>(transformer(take_unchecked()));
    }
    return Option<M>(None{});
  }

  /**
   * Gets value contained in option, or uses value_generator to create a value
   * if there is none present in the option.
   */
  [[nodiscard]] T unwrap_or(const auto &value_generator) {
    static_assert(
        std::is_same_v<decltype(value_generator()), T>,
        "Default value generator must return an instance of T (in Option<T>)");

    if (is_none())
      return value_generator();
    return take_unchecked();
  }

  /**
   * Gets vaue contained in option, or uses 'value' as a default if there is no
   * value in the option.
   */
  [[nodiscard]] T unwrap_or(T &&value) {
    if (is_none())
      return value;
    return take_unchecked();
  }

  /**
   * Takes value out of the option and returns it, will error if option is none
   */
  [[nodiscard]] T take_unchecked() {
    assert(is_some() && "Cannot take from a empty option.");
    T some = std::move(get_unchecked());
    value = None{};
    return some;
  }

  Option<Ref<T>> as_ref() const {
    if (is_none())
      return None{};
    return Option(Ref<T>(get_unchecked()));
  }

  friend std::ostream &operator<<(std::ostream &os, const Option &opt) {
    if (opt.is_none())
      return os << "None";
    return os << opt.get_unchecked();
  }

  [[nodiscard]] T &get_unchecked() { return std::get<T>(value); }

  [[nodiscard]] const T &get_unchecked() const { return std::get<T>(value); }

private:
  std::variant<T, None> value;
};

namespace opt {
template <typename T> Option<T> some(T &&from) noexcept {
  return Option<T>(std::move(from));
}

inline constexpr None none = {};

template <typename T> T unwrap(Option<T> &&from) {
  return from.take_unchecked();
}

template <typename T = unit> class Else {
  T data;

public:
  Else(T data) : data(std::move(data)) {}

  /**
   * 'Else' Branch for Conditional
   */
  void or_else(const auto &block) {
    if (data) {
      if constexpr (std::is_same_v<bool, T>) {
        block();
      } else
        block(data.take_unchecked());
    }
  }
};

/**
 * Runs block if the given option is not null and will destructure the
 * option, returns an optional else conditional
 *
 * Sample Use:
 *
 * Option<f32> o = 42.f;
 *
 * opt::if_some(std::move(o), [](auto some){
 *   std::cout << "o = " << some << std::endl;
 * });
 *
 * opt::if_some(std::move(o), [](auto some){
 *   std::cout << "o is some";
 * }).or_else([]{
 *  std::cout << "o is none";
 * });
 */
template <typename T> auto if_some(Option<T> &&from, const auto &block) {
  if (from.is_none()) {
    return Else<bool>(true);
  }
  block(opt::unwrap(std::forward<Option<T>>(from)));

  return Else<bool>(false);
}

template <typename T>
Else<Option<T>> if_none(Option<T> &&from, const auto &block) {
  if (from.is_none()) {
    block();
    return Else(std::forward<Option<T>>(from));
  }

  return Else<Option<T>>(None{});
}

} // namespace opt

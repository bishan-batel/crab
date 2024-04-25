//
// Created by bishan_ on 3/12/24.
//

// ReSharper disable  CppNonExplicitConvertingConstructor

// ReSharper disable CppDFAUnreachableCode
#pragma once

#include <type_traits>
#include <utility>
#include <variant>
#include <iostream>
#include <crab_type_traits.hpp>

#include "debug.hpp"

namespace crab {
  struct None {
    [[nodiscard]] bool operator==(const None &) const { return true; }
  };
}

template<typename T> requires crab::ref::is_valid_type<T>
class Ref;

template<typename T> requires crab::ref::is_valid_type<T>
class RefMut;

/**
 * Represents a value that could be None, this is almost
 * always a better alternative to using nullptrs.
 *
 * Tagged union type between T and unit
 */
template<typename T>
class Option final {
public:
  Option(const T &from) noexcept : value(T{from}) {}

  Option(T &&from) noexcept : value(std::move(from)) {}

  Option(crab::None) noexcept : Option() {}

  Option() noexcept : value(crab::None{}) {}

  Option &operator=(T &&from) {
    value = from;
    return *this;
  }

  Option &operator=(crab::None) {
    value = crab::None{};
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
    return std::holds_alternative<crab::None>(value);
  }

  /**
   * Takes value out of the option and returns it, will error if option is none
   */
  [[nodiscard]] T take_unchecked() {
    debug_assert(is_some(), "Cannot take value from a empty option.");
    T some = std::move(get_unchecked());
    value = crab::None{};
    return some;
  }

  Option<Ref<T> > as_ref() const {
    if (is_none())
      return crab::None{};
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
  std::variant<T, crab::None> value;
};

namespace crab {
  template<typename T>
  Option<T> some(T from) noexcept {
    return Option<T>(std::move(from));
  }

  template<typename T>
  Option<T> some(const T &from) noexcept {
    return Option<T>(from);
  }

  inline constexpr None none{};

  /**
   * Consumes given option and returns the contained value, will throw if none found
   * @param from Option to consume
   * @return
   */
  template<typename T>
  T unwrap(Option<T> &&from) {
    return from.take_unchecked();
  }
} // namespace opt

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
#include <crab/type_traits.hpp>

#include "crab/debug.hpp"

namespace crab {
  struct None {
    [[nodiscard]] bool operator==(const None &) const { return true; }
  };
}

template<typename T> requires crab::ref::is_valid_type<T>
class Ref;

template<typename T> requires crab::ref::is_valid_type<T>
class RefMut;

namespace crab::ref {
  template<typename T>
  struct decay_type {
    using type = T;
    using underlying_type = T;
    using identity = T;
    static constexpr auto is_const_ref = false;
    static constexpr auto is_ref = false;
  };

  template<typename T>
  struct decay_type<T&> {
    using type = RefMut<typename decay_type<T>::type>;
    using underlying_type = T;
    static constexpr auto is_const_ref = false;
    static constexpr auto is_ref = true;
  };

  template<typename T>
  struct decay_type<const T&> {
    using type = Ref<typename decay_type<T>::type>;
    using underlying_type = T;
    static constexpr auto is_const_ref = true;
    static constexpr auto is_ref = true;
  };
}

/**
 * Represents a value that could be None, this is almost
 * always a better alternative to using nullptrs.
 *
 * Tagged union type between T and unit
 */
template<typename T>
class Option final {
public:
  // using decay = crab::ref::decay_type<T>;
  // using Contained = typename decay::type;
  // using UnderlyingType = typename decay::underlying_type;
  using Contained = T;

  Option(const Contained &from) noexcept : value(Contained{from}) {}

  // Option(UnderlyingType &from) noexcept requires (decay::is_const_ref)
  //   : value(Contained{from}) {}
  //
  // Option(RefMut<UnderlyingType> from) noexcept requires (decay::is_const_ref)
  //   : value(Contained{from}) {}
  //
  // Option(Option<UnderlyingType&> from) noexcept requires (decay::is_const_ref)
  //   : value(unit{}) {
  //   if (from) {
  //     value = Contained{from.take_unchecked()};
  //   } else {
  //     value = crab::None{};
  //   }
  // }

  Option(Contained &&from) noexcept : value(std::move(from)) {}

  Option(crab::None) noexcept : Option() {}

  Option() noexcept : value(crab::None{}) {}

  Option& operator=(Contained &&from) {
    value = std::forward<Contained>(from);
    return *this;
  }

  Option& operator=(crab::None) {
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
    return std::holds_alternative<Contained>(value);
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
  [[nodiscard]] Contained take_unchecked() {
    debug_assert(is_some(), "Cannot take value from a empty option.");
    Contained some = std::move(get_unchecked());
    value = crab::None{};
    return some;
  }

  Option<const Contained&> as_ref() const {
    if (is_none())
      return crab::None{};
    return Option(Ref<Contained>(get_unchecked()));
  }

  friend std::ostream& operator<<(std::ostream &os, const Option &opt) {
    if (opt.is_none())
      return os << "None";
    return os << opt.get_unchecked();
  }

  [[nodiscard]] Contained& get_unchecked() { return std::get<Contained>(value); }

  [[nodiscard]] const Contained& get_unchecked() const { return std::get<Contained>(value); }

private:
  std::variant<Contained, crab::None> value;
};

namespace crab {
  template<typename T>
  Option<T> some(T from) noexcept {
    return Option<T>(std::move(from));
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

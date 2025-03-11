//
// Created by bishan_ on 3/12/24.
//

// ReSharper disable  CppNonExplicitConvertingConstructor

// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppNonExplicitConversionOperator
#pragma once

#include <concepts>
#include <crab/type_traits.hpp>
#include <iostream>
#include <type_traits>
#include <utility>
#include <variant>

#include "crab/debug.hpp"
#include "hash.hpp"

namespace crab {
  /**
   * @brief 0-sized* struct to give into Option<T> to create an empty Option
   */
  struct None {
    constexpr None() = default;

    [[nodiscard]] constexpr auto operator==(const None&) const -> bool {
      return true;
    }
  };
} // namespace crab

template<crab::ref::is_valid_type T>
class Ref;

template<crab::ref::is_valid_type T>
class RefMut;

template<typename T, typename E>
class Result;

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

  template<typename T>
  struct decay_type<Ref<T>> : decay_type<const T&> {};

  template<typename T>
  struct decay_type<RefMut<T>> : decay_type<T&> {};

  template<typename T>
  struct is_ref_type : std::false_type {};

  template<typename T>
  struct is_ref_type<Ref<T>> : std::true_type {};

  template<typename T>
  struct is_ref_mut_type : std::false_type {};

  template<typename T>
  struct is_ref_mut_type<RefMut<T>> : std::true_type {};
} // namespace crab::ref

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
  // using T = typename decay::type;
  // using UnderlyingType = typename decay::underlying_type;
  using Contained = T;

  using NestedT = T;
  static constexpr usize nested_depth{0};

  /**
   * @brief Create an option that wraps Some(T)
   */
  constexpr Option(const T& from) /*NOLINT(*explicit*)*/ noexcept:
      value(T{from}) {}

  // Option(UnderlyingType &from) noexcept requires (decay::is_const_ref)
  //   : value(T{from}) {}
  //
  // Option(RefMut<UnderlyingType> from) noexcept requires (decay::is_const_ref)
  //   : value(T{from}) {}
  //
  // Option(Option<UnderlyingType&> from) noexcept requires
  // (decay::is_const_ref)
  //   : value(unit{}) {
  //   if (from) {
  //     value = T{from.take_unchecked()};
  //   } else {
  //     value = crab::None{};
  //   }
  // }

  /**
   * @brief Create an option that wraps Some(T)
   */
  constexpr Option(T&& from) /*NOLINT(*explicit*)*/ noexcept:
      value{std::move(from)} {}

  /**
   * @brief Create an empty option
   */
  constexpr Option(crab::None none = {}) /*NOLINT(*explicit*)*/ noexcept:
      value{none} {}

  /**
   * @brief Reassign option to Some(T),
   * If this option previously contained Some(K), the previous value is
   * discarded and is replaced by Some(T)
   */
  constexpr auto operator=(T&& from) -> Option& {
    value = std::forward<T>(from);
    return *this;
  }

  /**
   * @brief Reassign option to None,
   * If this option previously contained Some(K), the previous value is
   * discarded and is replaced by Some(T)
   */
  constexpr auto operator=(crab::None) -> Option& {
    value = crab::None{};
    return *this;
  }

  /**
   * @brief Move an option
   */
  constexpr Option(Option&& from) /*NOLINT(*explicit*)*/ noexcept:
      value(std::exchange(from.value, crab::None{})) {}

  constexpr auto operator=(Option&& opt) noexcept -> Option& {
    value = std::exchange(opt.value, crab::None{});
    return *this;
  }

  constexpr Option(const Option&) = default;

  constexpr auto operator=(const Option&) -> Option& = default;

  constexpr ~Option() = default;

  // /**
  //  * @brief Whether this option has a contained value or not (None)
  //  */
  [[nodiscard]] constexpr explicit operator bool() const { return is_some(); }

  /**
   * @brief Whether this option contains a value
   */
  [[nodiscard]] constexpr auto is_some() const -> bool {
    return std::holds_alternative<T>(value);
  }

  /**
   * @brief Whether this option does not contain a value
   */
  [[nodiscard]] constexpr auto is_none() const -> bool {
    return std::holds_alternative<crab::None>(value);
  }

  /**
   * @brief Takes value out of the option and returns it, will error if option
   * is none, After this, the value has been 'taken out' of this option, after
   * this method is called this option is 'None'
   */
  [[deprecated("Prefer safer unwrap instead")]] [[nodiscard]] constexpr auto
  take_unchecked(
    [[maybe_unused]] const std::source_location loc =
      std::source_location::current()
  ) -> T {
    debug_assert_transparent(
      is_some(),
      "Cannot take value from a empty option.",
      loc
    );
    return std::get<T>(std::exchange(value, crab::None{}));
  }

  /**
   * @brief Converts a 'const Option<T>' into a Option<Ref<T>>, to give optional
   * access to the actual referenced value inside.
   */
  [[nodiscard]] constexpr auto as_ref() const -> Option<Ref<T>> {
    if (is_none()) {
      return crab::None{};
    }
    return Option<Ref<T>>{Ref<T>{get_unchecked()}};
  }

  /**
   * @brief Converts a 'const Option<T>' or 'Option<T>&' into a
   * Option<RefMut<T>>, to give optional access to the actual referenced value
   * inside.
   */
  [[nodiscard]] constexpr auto as_ref_mut() -> Option<RefMut<T>> {
    if (is_none()) {
      return crab::None{};
    }
    return Option<RefMut<T>>{RefMut<T>{get_unchecked()}};
  }

  friend constexpr auto operator<<(std::ostream& os, const Option& opt)
    -> std::ostream& {
    if (opt.is_none()) {
      return os << "None";
    }
    os << "Some(";
    os << opt.get_unchecked();
    os << ")";

    return os;
  }

  /**
   * @brief Consumes inner value and returns it, if this option was none this
   * will instead return a new default constructed T
   */
  [[nodiscard]]
  constexpr auto take_or_default() && -> T requires std::constructible_from<T>
  {
    return std::move(*this).take_or([]() { return T{}; });
  }

  /**
   * @brief Takes the contained value (like Option<T>::take_unchecked()) if
   * exists, else returns a default value
   * @param default_value
   */
  [[nodiscard]] constexpr auto take_or(T default_value) && -> T {
    return is_some() ? T{std::move(*this).unwrap()} : std::move(default_value);
  }

  /**
   * @brief Takes the contained value (like Option<T>::take_unchecked()) if
   * exists, else uses 'F' to compute & create a default value
   * @param default_generator Function to create the default value
   */
  template<std::invocable F>
  [[nodiscard]] constexpr auto take_or(F default_generator) && -> T {
    return is_some() ? T{std::move(*this).unwrap()} : T{default_generator()};
  }

  /**
   * @brief Is this option has a value, return a copy of that value. if
   * this opton was none then this returns a default constructed T
   */
  [[nodiscard]]
  constexpr auto get_or_default() const -> T
    requires std::constructible_from<T> and std::copyable<T>
  {
    return get_or([]() { return T{}; });
  }

  /**
   * @brief Gets the contained value if exists, else returns a default value
   * @param default_value
   */
  [[nodiscard]] constexpr auto get_or(T default_value) const -> T
    requires std::copy_constructible<T>
  {
    return is_some() ? T{get_unchecked()} : std::move(default_value);
  }

  /**
   * @brief Gets the contained value if exists, else computes a default value
   * with 'F' and returns
   * @param default_generator Function to create the default value
   */
  template<std::invocable F>
  [[nodiscard]] constexpr auto get_or(F default_generator) const -> T
    requires std::copy_constructible<T>
         and std::convertible_to<std::invoke_result_t<F>, T>
  {
    return is_some() ? T{get_unchecked()} : T{default_generator()};
  }

  /**
   * @brief Takes value out of the option and returns it, will error if option
   * is none, After this, the value has been 'taken out' of this option, after
   * this method is called this option is 'None'
   */
  [[nodiscard]] constexpr auto unwrap(
    [[maybe_unused]] const std::source_location loc =
      std::source_location::current()
  ) && -> T {
    debug_assert_transparent(is_some(), "Cannot unwrap a none option", loc);
    return std::get<T>(std::exchange(value, crab::None{}));
  }

  /**
   * @brief Returns a mutable reference to the contained Some value inside, if
   * this option is none this will panic & crash.
   */
  [[nodiscard]] constexpr auto get_unchecked(
    [[maybe_unused]] const std::source_location loc =
      std::source_location::current()
  ) -> T& {
    debug_assert_transparent(
      is_some(),
      "cannot get_unchecked a none option",
      loc
    );
    return std::get<T>(value);
  }

  /**
   * @brief Returns a const reference to the contained Some value inside, if
   * this option is none this will panic & crash.
   */
  [[nodiscard]] constexpr auto get_unchecked(
    [[maybe_unused]] const std::source_location loc =
      std::source_location::current()
  ) const -> const T& {
    debug_assert_transparent(
      is_some(),
      "cannot get_unchecked a none option",
      loc
    );
    return std::get<T>(value);
  }

  /**
   * @brief Creates a Result<T, E> from this given option, where "None" is
   * expanded to some error given.
   * @tparam E Error Type
   * @param error Error to replace an instance of None
   */
  template<typename E>
  [[nodiscard]] constexpr auto ok_or(E error) const -> Result<T, E>
    requires std::copy_constructible<T>
  {
    return is_some() ? Result<T, E>{get_unchecked()} : std::move(error);
  }

  /**
   * @brief Creates a Result<T, E> from this given option, where "None" is
   * expanded to some error given.
   * @tparam E Error Type
   * @param error_generator Function to generate an error to replace "None".
   */
  template<typename E, std::invocable F>
  [[nodiscard]] constexpr auto ok_or(F error_generator) const -> Result<T, E>
    requires std::copy_constructible<T>
         and std::convertible_to<std::invoke_result_t<F>, E>
  {
    return is_some() ? Result<T, E>{get_unchecked()} : error_generator();
  }

  /**
   * @brief Converts this Result<T, E> from this given option, where "None" is
   * expanded to some error given. This will invalidate the Option<T> after
   * call, just like every other Option::take_* function.
   * @tparam E Error Type
   * @param error Error to replace an instance of None
   */
  template<typename E>
  [[nodiscard]] constexpr auto take_ok_or(E error) && -> Result<T, E> {
    if (is_some()) {
      return Result<T, E>{std::move(*this).unwrap()};
    }
    return Result<T, E>(std::move(error));
  }

  /**
   * @brief Creates a Result<T, E> from this given option, where "None" is
   * expanded to some error given.
   * @tparam E Error Type
   * @param error_generator Function to generate an error to replace "None".
   */
  template<typename E, std::invocable F>
  [[nodiscard]] constexpr auto take_ok_or(F error_generator
  ) && -> Result<T, E> {
    if (is_some()) {
      return Result<T, E>{std::move(*this).unwrap()};
    }
    return Result<T, E>(error_generator());
  }

  /**
   * @brief Transform function to an Option<T> -> Option<K>, if this option is
   * None it will simply return None, but if it is Some(T), this will take the
   * value out of this option and call the given 'mapper' function with it, the
   * returned value is then wrapped and this function returns Some
   *
   * ex.
   *
   * assert(
   *  Option<f32>{crab::none}
   *  .map([](f32 x) { return static_cast<i32>(x); }) // returns Option<i32>
   *  .is_none()
   * );
   *
   * assert( Option<i32>{420}
   *  .map([](i32 x) { return std::tostring(x); }) // returns a Option<String>
   *  .take_unchecked() == "420"
   * );
   */
  template<std::invocable<T> F>
  [[nodiscard]] constexpr auto map(F mapper) && {
    using Returned = Option<crab::clean_invoke_result<F, T>>;
    if (is_some()) {
      return Returned{mapper(std::move(*this).unwrap())};
    }
    return Returned{};
  }

  /**
   * @brief Transform function to an Option<T> -> Option<K>, if this option is
   * None it will simply return None, but if it is Some(T), this will copy the
   * value of this option and call the given 'mapper' function with it, the
   * returned value is then wrapped and this function returns Some
   *
   * ex.
   *
   * assert(
   *  Option<f32>{crab::none}
   *  .map([](f32 x) { return static_cast<i32>(x); }) // returns Option<i32>
   *  .is_none()
   * );
   *
   * assert( Option<i32>{420}
   *  .map([](i32 x) { return std::tostring(x); }) // returns a Option<String>
   *  .take_unchecked() == "420"
   * );
   */
  template<std::invocable<T> F>
  requires std::copy_constructible<T>
  [[nodiscard]] constexpr auto map(F mapper) const& {
    return copied().map(mapper);
  }

  template<typename Into>
  requires std::convertible_to<T, Into> and std::copy_constructible<T>
  [[nodiscard]] constexpr auto map() const& {
    return copied().map([](T&& value) {
      return static_cast<Into>(std::forward<T>(value));
    });
  }

  template<typename Into>
  requires std::convertible_to<T, Into>
  [[nodiscard]] constexpr auto map() && {
    return std::move(*this).map([](T&& value) {
      return static_cast<Into>(std::forward<T>(value));
    });
  }

  /**
   * @brief Shorthand for calling .map(...).flatten()
   */
  template<std::invocable<T> F>
  [[nodiscard]] constexpr auto flat_map(F mapper) && {
    using Returned = crab::clean_invoke_result<F, T>;
    if (is_some()) {
      return Returned{mapper(std::move(*this).unwrap())};
    }
    return Returned{crab::None{}};
  }

  /**
   * @brief Shorthand for calling .map(...).flatten()
   */
  template<std::invocable<T> F>
  requires std::copy_constructible<T>
  [[nodiscard]] constexpr auto flat_map(F mapper) const& {
    using Returned = crab::clean_invoke_result<F, T>;

    if (is_some()) {
      return Returned{mapper(T{get_unchecked()})};
    }
    return Returned{crab::None{}};
  }

  /**
   * @brief If this option is of some type Option<Option<T>>, this will flatten
   * it to a single Option<T>
   */
  template<std::same_as<unit> = unit>
  requires std::constructible_from<T, crab::None>
  [[nodiscard]] constexpr auto flatten() && -> T {
    if (is_some()) {
      return std::move(*this).unwrap();
    }
    return crab::None{};
  }

  /**
   * @brief If this option is of some type Option<Option<T>>, this will flatten
   * it to a single Option<T>
   */
  template<std::same_as<unit> = unit>
  requires std::copy_constructible<T> and std::constructible_from<T, crab::None>
  [[nodiscard]] constexpr auto flatten() const& -> T {
    if (is_some()) {
      return T{get_unchecked()};
    }
    return crab::None{};
  }

  /**
   * @brief Copies this option and returns, use this before map if you do not
   * want to consume the option.
   */
  [[nodiscard]] constexpr auto copied() const -> Option
    requires std::copy_constructible<T>
  {
    if (is_some()) {
      return T{get_unchecked()};
    }

    return crab::None{};
  }

  /**
   * @brief Consumes this option, if this is some and passes the predicate it
   * will return Some, else None
   */
  template<std::predicate<const T&> F>
  [[nodiscard]] constexpr auto filter(F func) && -> Option {
    if (is_none()) {
      return crab::None{};
    }

    T value{std::move(*this).unwrap()};

    if (not static_cast<bool>(func(static_cast<const T&>(value)))) {
      return crab::None{};
    }

    return Option{std::move(value)};
  }

  /**
   * @brief Copys this option, if this is some and passes the predicate it will
   * return Some, else None
   */
  template<std::predicate<const T&> F>
  requires std::copy_constructible<T>
  [[nodiscard]] constexpr auto filter(F func) const& -> Option<T> {
    if (is_none()) {
      return crab::None{};
    }

    T value{get_unchecked()};

    if (not static_cast<bool>(func(static_cast<const T&>(value)))) {
      return crab::None{};
    }

    return Option{std::move(value)};
  }

  /**
   * @brief Copys this option, if this is some and passes the predicate it will
   * return Some, else None
   */
  template<std::predicate<const T&> F>
  requires std::copy_constructible<T>
  [[nodiscard]] constexpr auto filter(F func) & -> Option<T> {
    if (is_none()) {
      return crab::None{};
    }

    T value{get_unchecked()};

    if (not static_cast<bool>(func(static_cast<T&>(value)))) {
      return crab::None{};
    }

    return Option{std::move(value)};
  }

  /**
   * Combines many Option values into one if they are all Some,
   * if any of the options are empty, this returns an empty option.
   *
   *
   * ```cpp
   *
   * void print_sum(Option<i32> a, Option<i32> b) {
   *  a.zip(b);
   * }
   *
   * ````
   */
  template<typename... Vals>
  [[nodiscard]]
  constexpr auto zip(Option<Vals>... other) && -> Option<Tuple<T, Vals...>> {
    if (is_none()) {
      return crab::None{};
    }

    if ((... or other.is_none())) {
      return crab::None{};
    }

    return std::make_tuple(
      std::move(*this).unwrap(),
      std::move(other).unwrap()...
    );
  }

  ///
  /// Ordering Overloads
  ///

  template<typename S>
  [[nodiscard]]
  constexpr auto operator==(const Option<S>& other) const -> bool {
    static_assert(
      std::equality_comparable_with<T, S>,
      "Cannot equate to options if the inner types are not equatable"
    );
    if (is_none() or other.is_none()) {
      return is_none() == other.is_none();
    }

    return get_unchecked() == other.get_unchecked();
  }

  template<typename S>
  [[nodiscard]]
  constexpr auto operator!=(const Option<S>& other) const -> bool {
    static_assert(
      std::equality_comparable_with<T, S>,
      "Cannot equate to options if the inner types are not inverse equatable"
    );
    if (is_none() or other.is_none()) {
      return is_none() != other.is_none();
    }

    return get_unchecked() != other.get_unchecked();
  }

  template<typename S>
  [[nodiscard]]
  constexpr auto operator>(const Option<S>& other) const -> bool {
    static_assert(
      requires(const T& a, const S& b) {
        { a > b } -> std::convertible_to<bool>;
      },
      "Cannot compare to options if the inner types are not comparable with >"
    );

    if (is_none()) {
      return false;
    }

    if (other.is_none()) {
      return true;
    }

    return get_unchecked() > other.get_unchecked();
  }

  template<typename S>
  [[nodiscard]]
  constexpr auto operator<(const Option<S>& other) const -> bool {
    static_assert(
      requires(const T& a, const S& b) {
        { a < b } -> std::convertible_to<bool>;
      },
      "Cannot compare to options if the inner types are not comparable with <"
    );

    if (is_none()) {
      return other.is_some();
    }

    if (other.is_none()) {
      return true;
    }

    return get_unchecked() < other.get_unchecked();
  }

  template<typename S>
  [[nodiscard]]
  constexpr auto operator>=(const Option<S>& other) const -> bool {
    static_assert(
      requires(const T& a, const S& b) {
        { a >= b } -> std::convertible_to<bool>;
      },
      "Cannot compare to options if the inner types are not comparable with >="
    );

    if (is_none()) {
      return other.is_none();
    }

    if (other.is_none()) {
      return true;
    }

    return get_unchecked() >= other.get_unchecked();
  }

  template<typename S>
  [[nodiscard]]
  constexpr auto operator<=(const Option<S>& other) const -> bool {
    static_assert(
      requires(const T& a, const S& b) {
        { a <= b } -> std::convertible_to<bool>;
      },
      "Cannot compare to options if the inner types are not comparable with <="
    );

    if (is_none()) {
      return true;
    }

    if (other.is_none()) {
      return false;
    }

    return get_unchecked() <= other.get_unchecked();
  }

  template<typename S>
  [[nodiscard]]
  constexpr auto operator<=>(const Option<S>& other
  ) const -> std::partial_ordering {
    static_assert(
      std::three_way_comparable_with<T, S>,
      "Cannot compare to options if the inner types are not comparable with <=>"
    );

    if (is_none() and other.is_none()) {
      return std::partial_ordering::equivalent;
    }

    // None < Some
    if (is_none()) {
      return std::partial_ordering::less;
    }

    // Some(T) > None
    if (other.is_none()) {
      return std::partial_ordering::greater;
    }

    return static_cast<std::partial_ordering>(
      get_unchecked() <=> other.get_unchecked()
    );
  }

  /**
   * @brief Option<T> == None only if the former is none
   */
  [[nodiscard]] constexpr auto operator==(const crab::None&) const -> bool {
    return is_none();
  }

  /**
   * @brief Option<T> != None only if the former is some
   */
  [[nodiscard]] constexpr auto operator!=(const crab::None&) const -> bool {
    return is_some();
  }

  /**
   * @brief Option<T> > None only if the former is some
   */
  [[nodiscard]] constexpr auto operator>(const crab::None&) const -> bool {
    return is_some();
  }

  /**
   * @brief Option<T> >= None is always true
   */
  [[nodiscard]] constexpr auto operator>=(const crab::None&) const -> bool {
    return true;
  }

  /**
   * @brief Option<T> < None is never true
   */
  [[nodiscard]] constexpr auto operator<(const crab::None&) const -> bool {
    return false;
  }

  /**
   * @brief Option<T> <= None is never true
   */
  [[nodiscard]] constexpr auto operator<=(const crab::None&) const -> bool {
    return is_none();
  }

private:

  std::variant<T, crab::None> value;
};

namespace std {
  template<typename T>
  struct hash<Option<T>> /*NOLINT*/ {
    [[nodiscard]]
    auto operator()(const Option<T>& opt) const -> crab::hash_code {
      if (opt.is_none()) {
        return 0;
      }

      return crab::hash_together<usize, T>(1, opt.get_unchecked());
    }
  };
};

namespace crab {
  /**
   * @brief 'None' value type for use with Option<T>
   */
  inline constinit None none{};

  /**
   * @brief Creates an Option<T> from some value T
   */
  template<typename T>
  [[nodiscard]] constexpr auto some(T from) {
    return Option<std::remove_cvref_t<T>>{std::move(from)};
  }

  /**
   * @brief Maps a boolean to an option if it is true
   */
  template<std::invocable F>
  [[nodiscard]] constexpr auto then(const bool cond, F&& func) {
    if (not cond) {
      return Option<crab::clean_invoke_result<F>>{};
    }
    return Option<crab::clean_invoke_result<F>>{func()};
  }

  /**
   * @brief Maps a boolean to an option if it is false
   */
  template<std::invocable F>
  [[nodiscard]] constexpr auto unless(const bool cond, F&& func)
    -> Option<std::invoke_result_t<F>> {
    if (cond) {
      return Option<crab::clean_invoke_result<F>>{};
    }
    return Option<crab::clean_invoke_result<F>>{func()};
  }

  /**
   * @brief Consumes given option and returns the contained value, will throw if
   * none found
   * @param from Option to consume
   */
  template<typename T>
  constexpr auto unwrap(Option<T>&& from) -> T {
    return std::move<T>(from).unwrap();
  }

  namespace option {
    struct fallible final {
      constexpr fallible() = default;

      // Identity
      constexpr auto operator()(auto tuple) const { return tuple; }

      // Pass with Result<T, E>
      template<std::invocable F, std::invocable... Rest>
      constexpr auto operator()(
        // Tuple : Result<std:tuple<...>, Error>
        auto tuple,
        F function,
        Rest... other_functions
      ) const requires option_type<decltype(function())>
      {
        // tuple.take_unchecked();

        using O = decltype(function());
        using Contained = typename O::Contained;

        // static_assert(std::same_as<typename R::ErrType, Error>, "Cannot have
        // multiple types of errors in fallible chain.");

        using ReturnOk = decltype(std::tuple_cat(
          std::move(tuple).unwrap(),
          std::make_tuple(function().unwrap())
        ));

        // using Return = std::invoke_result_t<decltype(operator()),
        // Result<ReturnOk, Error>, Rest...>;
        using Return = decltype(operator()(
          Option<ReturnOk>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::make_tuple(function().unwrap())
          )},
          other_functions...
        ));

        if (tuple.is_none()) {
          return Return{crab::none};
        }

        Option<Contained> result = function();

        if (result.is_none()) {
          return Return{crab::none};
        }

        return operator()(
          Option<ReturnOk>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::make_tuple(std::move(result).unwrap())
          )},
          other_functions...
        );
      }

      template<std::invocable F, std::invocable... Rest>
      constexpr auto operator()(
        // Tuple : Result<std:tuple<...>, Error>
        auto tuple,
        F function,
        Rest... other_functions
      ) const requires(not option_type<decltype(function())>)
      {
        // tuple.take_unchecked();

        using O = decltype(function());

        using ReturnOk = decltype(std::tuple_cat(
          std::move(tuple).unwrap(),
          std::make_tuple(function())
        ));

        using Return = decltype(operator()(
          Option<ReturnOk>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::make_tuple(function())
          )},
          other_functions...
        ));

        if (tuple.is_none()) {
          return Return{crab::none};
        }

        O result = function();

        return operator()(
          Option<ReturnOk>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::make_tuple(std::move(result))
          )},
          other_functions...
        );
      }
    };

    template<typename T>
    struct decay_fallible {
      using type = T;
    };

    template<typename T>
    struct decay_fallible<Option<T>> {
      using type = T;
    };

    template<std::invocable F>
    using decay_fallible_function =
      typename decay_fallible<std::invoke_result_t<F>>::type;
  } // namespace option

  template<std::invocable... F>
  constexpr auto fallible(const F... fallible
  ) -> Option<std::tuple<option::decay_fallible_function<F>...>> {
    return option::fallible{}(Option{std::make_tuple()}, fallible...);
  }
} // namespace crab

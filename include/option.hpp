//
// Created by bishan_ on 3/12/24.
//

// ReSharper disable  CppNonExplicitConvertingConstructor

// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppNonExplicitConversionOperator
#pragma once

#include <crab/type_traits.hpp>
#include <iostream>
#include <type_traits>
#include <utility>
#include <variant>

#include "crab/debug.hpp"

namespace crab {
  struct None {
    [[nodiscard]] constexpr auto operator==(const None &) const -> bool { return true; }
  };
} // namespace crab

template<typename T>
  requires crab::ref::is_valid_type<T>
class Ref;

template<typename T>
  requires crab::ref::is_valid_type<T>
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
  struct decay_type<T &> {
    using type = RefMut<typename decay_type<T>::type>;
    using underlying_type = T;
    static constexpr auto is_const_ref = false;
    static constexpr auto is_ref = true;
  };

  template<typename T>
  struct decay_type<const T &> {
    using type = Ref<typename decay_type<T>::type>;
    using underlying_type = T;
    static constexpr auto is_const_ref = true;
    static constexpr auto is_ref = true;
  };
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
  // using Contained = typename decay::type;
  // using UnderlyingType = typename decay::underlying_type;
  using Contained = T;

  using NestedContained = T;
  static constexpr usize nested_depth{0};

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

  Option(Contained &&from) noexcept : value{std::move(from)} {}

  Option(crab::None none = {}) noexcept : value{none} {}

  auto operator=(Contained &&from) -> Option & {
    value = std::forward<Contained>(from);
    return *this;
  }

  auto operator=(crab::None) -> Option & {
    value = crab::None{};
    return *this;
  }

  /**
   * @brief Whether this option has a contained value or not (None)
   */
  [[nodiscard]] operator bool() const { return is_some(); }

  /**
   * @brief Whether this option contains a value */
  [[nodiscard]] auto is_some() const -> bool { return std::holds_alternative<Contained>(value); }

  /**
   * @brief Whether this option does not contain a value
   */
  [[nodiscard]] auto is_none() const -> bool { return std::holds_alternative<crab::None>(value); }

  /**
   * @brief Takes value out of the option and returns it, will error if option is none
   */
  [[nodiscard]] auto take_unchecked() -> Contained {
#if DEBUG
    debug_assert(is_some(), "Cannot take value from a empty option.");
#endif
    return std::get<Contained>(std::exchange(value, crab::None{}));
  }

  /**
   * @brief Converts a 'const Option<T>&' into a Option<Ref<T>>, to give optional access to the actual
   * referenced value inside.
   */
  auto as_ref() const -> Option<Ref<Contained>> {
    if (is_none())
      return crab::None{};
    return Option{Ref<Contained>{get_unchecked()}};
  }

  /**
   * @brief Converts a 'const Option<T>&' or 'Option<T>&' into a Option<RefMut<T>>, to give optional access to the
   * actual referenced value inside.
   */
  auto as_ref_mut() -> Option<RefMut<Contained>> {
    if (is_none())
      return crab::None{};
    return Option{RefMut<Contained>{get_unchecked()}};
  }

  friend auto operator<<(std::ostream &os, const Option &opt) -> std::ostream & {
    if (opt.is_none())
      return os << "None";
    return os << opt.get_unchecked();
  }

  /**
   * @brief Gets the contained value if exists, else returns a default value
   * @param default_value
   */
  [[nodiscard]] auto get_or(const Contained default_value) const -> Contained
    requires std::is_copy_constructible_v<Contained>
  {
    return this->get_or([default_value] { return default_value; });
  }

  /**
   * @brief Gets the contained value if exists, else computes a default value with 'F' and returns
   * @param default_generator Function to create the default value
   */
  template<std::invocable F>
  [[nodiscard]] auto get_or(const F default_generator) const -> Contained
    requires std::is_copy_constructible_v<Contained> and std::convertible_to<std::invoke_result_t<F>, Contained>
  {
    return is_some() ? Contained{get_unchecked()} : Contained{default_generator()};
  }

  /**
   * @brief Takes the contained value (like Option<T>::take_unchecked()) if exists, else returns a default value
   * @param default_value
   */
  [[nodiscard]] auto take_or(const Contained default_value) -> Contained {
    return this->take_or([default_value] { return default_value; });
  }

  /**
   * @brief Takes the contained value (like Option<T>::take_unchecked()) if exists, else uses 'F' to compute & create a
   * default value
   * @param default_generator Function to create the default value
   */
  template<std::invocable F>
  [[nodiscard]] auto take_or(const F default_generator) -> Contained
    requires std::is_copy_constructible_v<Contained> and std::convertible_to<std::invoke_result_t<F>, Contained>
  {
    return is_some() ? Contained{get_unchecked()} : Contained{default_generator()};
  }

  [[nodiscard]] auto get_unchecked() -> Contained & { return std::get<Contained>(value); }

  [[nodiscard]] auto get_unchecked() const -> const Contained & { return std::get<Contained>(value); }

  /**
   * @brief Creates a Result<T, E> from this given option, where "None" is expanded to some error given.
   * @tparam E Error Type
   * @param error Error to replace an instance of None
   */
  template<typename E>
  auto ok_or(E error) const -> Result<Contained, E>
    requires std::copy_constructible<Contained>
  {
    return this->ok_or([error] { return error; });
  }

  /**
   * @brief Creates a Result<T, E> from this given option, where "None" is expanded to some error given.
   * @tparam E Error Type
   * @param error_generator Function to generate an error to replace "None".
   */
  template<typename E, std::invocable F>
  auto ok_or(F error_generator) const -> Result<Contained, E>
    requires std::copy_constructible<Contained> and std::convertible_to<std::invoke_result_t<F>, E>
  {
    if (is_some()) {
      return get_unchecked();
    }
    return error_generator();
  }

  /**
   * @brief Converts this Result<T, E> from this given option, where "None" is expanded to some error given.
   * This will invalidate the Option<T> after call, just like every other Option::take_* function.
   * @tparam E Error Type
   * @param error Error to replace an instance of None
   */
  template<typename E>
  auto take_ok_or(E error) -> Result<Contained, E> {
    return this->ok_or([error] { return error; });
  }

  /**
   * @brief Creates a Result<T, E> from this given option, where "None" is expanded to some error given.
   * @tparam E Error Type
   * @param error_generator Function to generate an error to replace "None".
   */
  template<typename E, std::invocable F>
  auto take_ok_or(F error_generator) -> Result<Contained, E>
    requires std::copy_constructible<Contained> and std::convertible_to<std::invoke_result_t<F>, E>
  {
    if (is_some()) {
      return take_unchecked();
    }
    return error_generator();
  }

  template<std::invocable<Contained &> F>
  auto if_some(const F clause) -> Option<Contained> & {
    if (is_some()) {
      clause(get_unchecked());
    }

    return *this;
  }

  template<std::invocable<const Contained &> F>
  auto if_some(const F clause) const -> const Option<Contained> & {
    if (is_some()) {
      clause(get_unchecked());
    }
    return *this;
  }

  template<std::invocable F>
  auto if_none(const F clause) -> Option<Contained> & {
    if (is_none()) {
      clause();
    }
    return *this;
  }

  template<std::invocable F>
  auto if_none(const F clause) const -> const Option<Contained> & {
    if (is_none()) {
      clause();
    }
    return *this;
  }

  template<std::invocable<Contained> F>
  auto map(const F mapper) -> Option<decltype(mapper(take_unchecked()))> {
    if (is_some())
      return mapper(take_unchecked());
    return {};
  }

  template<std::invocable<Contained> F>
  auto flat_map(const F mapper) -> Option<decltype(mapper(take_unchecked()).take_unchecked())> {
    if (is_some()) {
      return mapper(take_unchecked());
    }
    return crab::None{};
  }

  template<std::same_as<unit> = unit>
  auto flatten() -> decltype(take_unchecked()) {
    if (is_some()) {
      return take_unchecked();
    }
    return decltype(take_unchecked()){};
  }

private:
  std::variant<Contained, crab::None> value;
};

namespace crab {

  namespace option {
    template<typename T>
    struct is_option_type : std::false_type {};

    template<typename T>
    struct is_option_type<Option<T>> : std::true_type {};
  } // namespace option

  template<typename T>
  concept option_type = option::is_option_type<T>::value;

  /**
   * @brief Creates an Option<T> from some value T
   */
  template<typename T>
  auto some(T from) -> Option<T> {
    return Option<T>{std::move(from)};
  }

  /**
   * @brief 'None' value type for use with Option<T>
   */
  inline constexpr None none{};

  /**
   * @brief Consumes given option and returns the contained value, will throw if none found
   * @param from Option to consume
   */
  template<typename T>
  auto unwrap(Option<T> &&from) -> T {
    return from.take_unchecked();
  }

  namespace option {
    struct fallible final {
      constexpr fallible() = default;

      // Identity
      __always_inline auto operator()(auto tuple) const { return tuple; }

      // Pass with Result<T, E>
      template<std::invocable F, std::invocable... Rest>
      __always_inline auto operator()(
        // Tuple : Result<std:tuple<...>, Error>
        auto tuple,
        const F function,
        const Rest... other_functions
      ) const
        requires option_type<decltype(function())>
      {
        // tuple.take_unchecked();

        using O = decltype(function());
        using Contained = typename O::Contained;

        // static_assert(std::same_as<typename R::ErrType, Error>, "Cannot have multiple types of errors in fallible
        // chain.");

        using ReturnOk = decltype(std::tuple_cat(tuple.take_unchecked(), std::make_tuple(function().take_unchecked())));

        // using Return = std::invoke_result_t<decltype(operator()), Result<ReturnOk, Error>, Rest...>;
        using Return = decltype(operator()(
          Option<ReturnOk>{std::tuple_cat(tuple.take_unchecked(), std::make_tuple(function().take_unchecked()))},
          other_functions...
        ));

        if (tuple.is_none())
          return Return{crab::none};

        Option<Contained> result = function();

        if (result.is_none())
          return Return{crab::none};

        return operator()(
          Option<ReturnOk>{std::tuple_cat(tuple.take_unchecked(), std::make_tuple(result.take_unchecked()))},
          other_functions...
        );
      }

      template<std::invocable F, std::invocable... Rest>
      __always_inline auto operator()(
        // Tuple : Result<std:tuple<...>, Error>
        auto tuple,
        const F function,
        const Rest... other_functions
      ) const
        requires(not option_type<decltype(function())>)
      {
        // tuple.take_unchecked();

        using O = decltype(function());

        using ReturnOk = decltype(std::tuple_cat(tuple.take_unchecked(), std::make_tuple(function())));

        using Return = decltype(operator()(
          Option<ReturnOk>{std::tuple_cat(tuple.take_unchecked(), std::make_tuple(function()))}, other_functions...
        ));

        if (tuple.is_none())
          return Return{crab::none};

        O result = function();

        return operator()(
          Option<ReturnOk>{std::tuple_cat(tuple.take_unchecked(), std::make_tuple(std::move(result)))},
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
    using decay_fallible_function = typename decay_fallible<std::invoke_result_t<F>>::type;
  } // namespace option

  template<std::invocable... F>
  auto fallible(const F... fallible) -> Option<std::tuple<option::decay_fallible_function<F>...>> {
    constexpr static crab::option::fallible stateless{};
    return stateless(Option<std::tuple<>>{std::make_tuple()}, fallible...);
  }
} // namespace crab

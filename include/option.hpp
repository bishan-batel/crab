//
// Created by bishan_ on 3/12/24.
//

// ReSharper disable  CppNonExplicitConvertingConstructor

// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppNonExplicitConversionOperator
#pragma once

#include <concepts>
#include <crab/type_traits.hpp>
#include <functional>
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

template<typename T>
class Option;

namespace crab::option {

  template<typename T>
  struct Inner {
  private:

    std::array<u8, sizeof(T)> bytes;
    bool use_flag{false};

  public:

    constexpr Inner() noexcept: use_flag{false} {}

    explicit constexpr Inner( //
      T&& value
    ) noexcept(std::is_copy_constructible_v<T>):
        use_flag{true} {
      std::construct_at<T, T&&>(address(), std::forward<T>(value));
    }

    explicit constexpr Inner(const T& value
    ) noexcept(std::is_copy_constructible_v<T>)
      requires std::is_copy_assignable_v<T>
        : use_flag{true} {
      std::construct_at<T, const T&>(address(), value);
    }

    constexpr Inner( //
      Inner&& from
    ) noexcept(std::is_nothrow_move_constructible_v<T>) {
      if (not from.in_use()) {
        return;
      }
      use_flag = true;
      std::construct_at<T, T&&>(address(), std::forward<Inner>(from).value());
    }

    constexpr Inner( //
      const Inner& from
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
      requires(std::copy_constructible<T>)
    {
      if (not from.in_use()) {
        return;
      }

      use_flag = true;

      std::construct_at<T, const T&>(address(), from.value());
    }

    constexpr auto operator=( //
      T&& from
    ) noexcept(std::is_nothrow_move_assignable_v<T> and std::is_nothrow_move_constructible_v<T>)
      -> Inner& {
      if (not in_use()) {
        use_flag = true;

        std::construct_at<T, T&&>(address(), std::forward<T>(from));

        return *this;
      }

      value() = std::forward<T>(from);

      return *this;
    }

    constexpr auto operator=( //
      const T& from
    ) noexcept(std::is_nothrow_copy_assignable_v<T> and std::is_nothrow_copy_constructible_v<T>)
      -> Inner& requires std::is_copy_assignable_v<T>
    {
      if (not in_use()) {
        use_flag = true;

        std::construct_at<T, const T&>(address(), from);

        return *this;
      }

      value() = from;

      return *this;
    }

    constexpr auto operator=( //
      Inner&& from
    ) noexcept(std::is_nothrow_move_assignable_v<T> and std::is_nothrow_move_constructible_v<T>)
      -> Inner& {
      if (&from == this) {
        return *this;
      }

      if (not from.in_use()) {
        destroy();
        return *this;
      }

      if (in_use()) {
        value() = std::move(from).value();
      } else {
        use_flag = true;
        std::construct_at<T, T&&>(address(), std::move(from).value());
      }
      return *this;
    }

    constexpr auto operator=( //
      const Inner& from
    ) noexcept(std::is_nothrow_copy_assignable_v<T>) -> Inner& {
      if (&from == this) {
        return *this;
      }

      if (not from.in_use()) {
        destroy();
        return *this;
      }

      if (in_use()) {
        value() = from.value();
      } else {
        use_flag = true;
        std::construct_at<T, const T&>(address(), from.value());
      }

      return *this;
    }

    constexpr ~Inner() {
      if (in_use()) {
        std::destroy_at(address());
      }
    }

    [[nodiscard]] constexpr auto in_use() const -> bool { return use_flag; }

    [[nodiscard]] constexpr auto value() const& -> const T& {
      return *address();
    }

    [[nodiscard]] constexpr auto value() & -> T& { return *address(); }

    [[nodiscard]] constexpr auto value() && -> T {
      return std::move(*address());
    }

    constexpr auto destroy() -> void {
      if (in_use()) {
        std::destroy_at(address());
        use_flag = false;
      }
    }

    [[nodiscard]] constexpr auto address() -> T* {
      return reinterpret_cast<T*>(&bytes[0]);
    }

    [[nodiscard]] constexpr auto address() const -> const T* {
      return reinterpret_cast<const T*>(&bytes[0]);
    }
  };

  template<typename T>
  class GenericStorage {
    template<typename S>
    friend class Option;

  public:

    constexpr GenericStorage() = default;

    constexpr explicit GenericStorage(T&& value):
        inner{std::forward<T>(value)} {}

    constexpr explicit GenericStorage(const T& value)
      requires std::is_copy_constructible_v<T>
        : inner{value} {}

    constexpr explicit GenericStorage(const crab::None&): GenericStorage{} {}

    auto operator=(T&& value) -> GenericStorage& {
      inner = std::forward<T>(value);
      return *this;
    }

    auto operator=( //
      const T& value
    ) -> GenericStorage& requires std::is_copy_assignable_v<T>
    {
      inner = value;
      return *this;
    }

    auto operator=(const crab::None&) -> GenericStorage& {
      inner.destroy();
      return *this;
    }

    [[nodiscard]] constexpr auto value() const& -> const T& {
      return inner.value();
    }

    [[nodiscard]] constexpr auto value() & -> T& { return inner.value(); }

    [[nodiscard]] constexpr auto value() && -> T {
      return std::move(inner).value();
    }

    [[nodiscard]] constexpr auto in_use() const -> bool {
      return inner.in_use();
    }

  private:

    Inner<T> inner;
  };

  template<typename T>
  struct PtrStorage {
    template<typename S>
    friend class Option;

  public:

    constexpr explicit PtrStorage(T& value): inner{&value} {}

    constexpr explicit PtrStorage(const crab::None& = crab::None{}):
        inner{nullptr} {}

    auto operator=(T& value) -> PtrStorage& {
      inner = &value;
      return *this;
    }

    auto operator=(const crab::None&) -> PtrStorage& {
      inner = nullptr;
      return *this;
    }

    [[nodiscard]] constexpr auto value() const& -> T& { return *inner; }

    [[nodiscard]] constexpr auto value() & -> T& { return *inner; }

    [[nodiscard]] constexpr auto value() && -> std::remove_const_t<T> {
      return std::move(*inner);
    }

    [[nodiscard]] constexpr auto in_use() const -> bool {
      return inner != nullptr;
    }

  private:

    T* inner;
  };

  template<typename T>
  struct storage_selector {
    using type = GenericStorage<T>;
  };

  template<typename T>
  struct storage_selector<T&> {
    using type = PtrStorage<T>;
  };

  template<typename T>
  struct storage_selector<const T&> {
    using type = PtrStorage<const T>;
  };

  template<typename T>
  using Storage = std::conditional_t<
    std::is_reference_v<T>,
    GenericStorage<std::reference_wrapper<std::remove_reference_t<T>>>,
    GenericStorage<T>>;
  // using Storage = typename storage_selector<T>::type;

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

  using Contained = T;

  using NestedT = T;
  static constexpr usize nested_depth{0};

  static constexpr bool is_ref = std::is_reference_v<T>;
  static constexpr bool is_const_ref = is_ref and std::is_const_v<T>;
  static constexpr bool is_mut_ref = is_ref and not std::is_const_v<T>;

  // NOLINTBEGIN(*explicit*)
  /**
   * @brief Create an option that wraps Some(T)
   */
  constexpr Option(const T& from) noexcept requires(not is_ref)
      : value{from} {}

  /**
   * @brief Create an option that wraps Some(T)
   */
  constexpr Option(T&& from) noexcept: value{std::forward<T>(from)} {}

  /**
   * @brief Create an empty option
   */
  constexpr Option(crab::None none = {}) noexcept: value{none} {}

  [[nodiscard]] constexpr operator Option<RefMut<std::remove_cvref_t<T>>>()
    requires(is_mut_ref)
  {
    return map<RefMut<std::remove_cvref_t<T>>>();
  }

  [[nodiscard]] constexpr operator Option<Ref<std::remove_cvref_t<T>>>()
    requires(is_ref)
  {
    return map<Ref<std::remove_cvref_t<T>>>();
  }

  // NOLINTEND(*explicit*)

  /**
   * @brief Reassign option to Some(T),
   * If this option previously contained Some(K), the previous value is
   * discarded and is replaced by Some(T)
   */
  constexpr auto operator=(T&& from) -> Option& requires(not is_ref)
  {
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
      value{std::move(from.value)} {}

  constexpr auto operator=(Option&& opt) noexcept -> Option& {
    value = std::move(opt.value);
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
    return value.in_use();
  }

  /**
   * @brief Whether this option does not contain a value
   */
  [[nodiscard]] constexpr auto is_none() const -> bool {
    return not value.in_use();
  }

  /**
   * @brief Converts a 'const Option<T>' into a Option<Ref<T>>, to give optional
   * access to the actual referenced value inside.
   */
  [[nodiscard]] constexpr auto as_ref() const requires(not is_ref)
  {
    if (is_none()) {
      return Option<const T&>{};
    }
    return Option<const T&>{get_unchecked()};
  }

  /**
   * @brief Converts a 'const Option<T>' or 'Option<T>&' into a
   * Option<T&>, to give optional access to the actual referenced value
   * inside.
   */
  [[nodiscard]] constexpr auto as_ref_mut() requires(not is_ref)
  {
    if (is_none()) {
      return Option<T&>{};
    }
    return Option<T&>{get_unchecked()};
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
  [[nodiscard]] constexpr auto take_or(T&& default_value) && -> T {
    return is_some() ? T{std::move(*this).unwrap()}
                     : std::forward<Contained>(default_value);
  }

  /**
   * @brief Takes the contained value (like Option<T>::take_unchecked()) if
   * exists, else uses 'F' to compute & create a default value
   * @param default_generator Function to create the default value
   */
  template<std::invocable F>
  [[nodiscard]] constexpr auto take_or(F default_generator) && -> T {
    return is_some() ? T{std::move(*this).unwrap()}
                     : T{std::invoke(default_generator)};
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
    return is_some() ? T{get_unchecked()} : T{std::invoke(default_generator)};
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
    return std::exchange(value, crab::None{}).value();
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

    return value.value();
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
    return value.value();
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
    return is_some() ? Result<T, E>{get_unchecked()}
                     : std::invoke(error_generator);
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
    return Result<T, E>(std::invoke(error_generator));
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
    using Returned = Option<std::invoke_result_t<F, T>>;
    if (is_some()) {
      return Returned{std::invoke(mapper, std::move(*this).unwrap())};
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
    return copied().template map<Into>();
  }

  template<typename Into>
  requires std::convertible_to<T, Into>
  [[nodiscard]] constexpr auto map() && {
    return std::move(*this).map([](T&& value) -> Into {
      return static_cast<Into>(std::forward<Contained>(value));
    });
  }

  /**
   * @brief Shorthand for calling .map(...).flatten()
   */
  template<std::invocable<T> F>
  [[nodiscard]] constexpr auto flat_map(F mapper) && {
    using Returned = crab::clean_invoke_result<F, T>;
    if (is_some()) {
      return Returned{std::invoke(mapper, std::move(*this).unwrap())};
    }
    return Returned{crab::None{}};
  }

  /**
   * @brief Shorthand for calling .map(...).flatten()
   */
  template<std::invocable<T> F>
  requires std::copy_constructible<T>
  [[nodiscard]] constexpr auto flat_map(F mapper) const& {
    return copied().flat_map(std::forward<F>(mapper));
  }

  /**
   * @brief Equivalent of flat_map but for the 'None' type
   */
  template<std::invocable F>
  [[nodiscard]] constexpr auto or_else(F mapper) && {
    using Returned = crab::clean_invoke_result<F, T>;

    if (is_some()) {
      return Returned{std::move(*this).unwrap()};
    }

    return Returned{std::invoke(mapper)};
  }

  /**
   * @brief Equivalent of flat_map but for the 'None' type
   */
  template<std::invocable F>
  requires std::copy_constructible<T>
  [[nodiscard]] constexpr auto or_else(F mapper) const& {
    return copied().or_else(std::forward<F>(mapper));
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
  [[nodiscard]] constexpr auto filter(F predicate) && -> Option {
    if (is_none()) {
      return crab::None{};
    }

    T value{std::move(*this).unwrap()};

    const bool passed{
      static_cast<bool>(std::invoke(predicate, static_cast<const T&>(value))),
    };

    if (not passed) {
      return crab::None{};
    }

    return Option{std::forward<Contained>(value)};
  }

  /**
   * @brief Copys this option, if this is some and passes the predicate it
   * will return Some, else None
   */
  template<std::predicate<const T&> F>
  requires std::copy_constructible<T>
  [[nodiscard]] constexpr auto filter(F predicate) const& -> Option<T> {
    if (is_none()) {
      return crab::None{};
    }

    T value{get_unchecked()};

    const bool passed{
      static_cast<bool>(std::invoke(predicate, static_cast<const T&>(value))),
    };

    if (not passed) {
      return crab::None{};
    }

    return Option{std::forward<Contained>(value)};
  }

  /**
   * @brief Copys this option, if this is some and passes the predicate it
   * will return Some, else None
   */
  template<std::predicate<const T&> F>
  requires std::copy_constructible<T>
  [[nodiscard]] constexpr auto filter(F predicate) & -> Option<T> {
    if (is_none()) {
      return crab::None{};
    }

    T value{get_unchecked()};

    const bool passed{
      static_cast<bool>(std::invoke(predicate, static_cast<T&>(value))),
    };

    if (not passed) {
      return crab::None{};
    }

    return Option{std::forward<Contained>(value)};
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

    return std::tuple<T, Vals...>{
      std::move(*this).unwrap(),
      std::move(other).unwrap()...
    };
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
      "Cannot compare to options if the inner types are not comparable with "
      ">="
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
      "Cannot compare to options if the inner types are not comparable with "
      "<="
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
      "Cannot compare to options if the inner types are not comparable with "
      "<=>"
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

  crab::option::Storage<T> value;
};

template<typename T>
constexpr auto operator<<(std::ostream& os, const Option<T>& opt)
  -> std::ostream& {
  if (opt.is_none()) {
    return os << "None";
  }
  os << "Some(";
  if constexpr (requires { os << opt.get_unchecked(); }) {
    os << opt.get_unchecked();
  } else {
    os << typeid(T).name();
  }
  os << ")";

  return os;
}

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
      return Option<std::invoke_result_t<F>>{};
    }
    return Option<std::invoke_result_t<F>>{std::invoke(func)};
  }

  /**
   * @brief Maps a boolean to an option if it is false
   */
  template<std::invocable F>
  [[nodiscard]] constexpr auto unless(const bool cond, F&& func)
    -> Option<std::invoke_result_t<F>> {
    if (cond) {
      return Option<std::invoke_result_t<F>>{};
    }
    return Option<std::invoke_result_t<F>>{std::invoke(func)};
  }

  /**
   * @brief Consumes given option and returns the contained value, will throw
   * if none found
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

        // static_assert(std::same_as<typename R::ErrType, Error>, "Cannot
        // have multiple types of errors in fallible chain.");

        using ReturnOk = decltype(std::tuple_cat(
          std::move(tuple).unwrap(),
          std::tuple<Contained>{std::invoke(function).unwrap()}
        ));

        // using Return = std::invoke_result_t<decltype(operator()),
        // Result<ReturnOk, Error>, Rest...>;
        using Return = decltype(operator()(
          Option<ReturnOk>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::tuple<Contained>(std::invoke(function).unwrap())
          )},
          other_functions...
        ));

        if (tuple.is_none()) {
          return Return{crab::none};
        }

        Option<Contained> result = std::invoke(function);

        if (result.is_none()) {
          return Return{crab::none};
        }

        return operator()(
          Option<ReturnOk>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::tuple<Contained>(std::move(result).unwrap())
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
      ) const requires(not option_type<decltype(std::invoke(function))>)
      {
        // tuple.take_unchecked();

        using O = decltype(std::invoke(function));

        using ReturnOk = decltype(std::tuple_cat(
          std::move(tuple).unwrap(),
          std::tuple<O>(std::invoke(function))
        ));

        using Return = decltype(operator()(
          Option<ReturnOk>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::tuple<O>(std::invoke(function))
          )},
          other_functions...
        ));

        if (tuple.is_none()) {
          return Return{crab::none};
        }

        O result = std::invoke(function);

        return operator()(
          Option<ReturnOk>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::tuple<O>(std::move(result))
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
  constexpr auto fallible(const F... fallible) {
    return option::fallible{}(Option{std::make_tuple()}, fallible...);
  }
} // namespace crab

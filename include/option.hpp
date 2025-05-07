/**
 * Created by bishan_ on 3/12/24.
 */

// Warning guards:

// ReSharper disable  CppNonExplicitConvertingConstructor
// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppNonExplicitConversionOperator
// NOLINTBEGIN(*explicit*)

#pragma once

#include <concepts>
#include <crab/type_traits.hpp>
#include <cstddef>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>

#include "crab/debug.hpp"
#include "hash.hpp"

namespace crab {
  /**
   * @brief 0-sized* struct to give into Option<T> to create an empty Option
   */
  struct None final {
    /**
     * Equality comparison for none, None is a unit type therefore this will
     * always return true.
     */
    [[nodiscard]] consteval auto operator==(const None&) const -> bool {
      return true;
    }
  };

  /**
   * @brief 'None' value type for use with Option<T>
   */
  inline constexpr None none{};

} // namespace crab

#if CRAB_OPTION_STD_VARIANT
#include <variant>
#endif

/**
 * Helper utilities for implementation of Option<T>, namely the
 * internal storage method
 */
namespace crab::option::helper {

#if CRAB_OPTION_STD_VARIANT
  /**
   * Generic tagged union storage for Option<T>, used as a
   * thin abstraction over std::variant (or a custom implementation later)
   */
  template<typename T>
  struct GenericStorage {
    /**
     * Default initialises to none
     */
    constexpr GenericStorage(): inner{None{}} {}

    /**
     * Initialise to Some(value)
     */
    constexpr explicit GenericStorage(T&& value):
        inner{std::forward<T>(value)} {}

    /**
     * Copy initialise to Some(value)
     */
    constexpr explicit GenericStorage(const T& value)
      requires std::copy_constructible<T>
        : inner{value} {}

    constexpr explicit GenericStorage(const crab::None&): GenericStorage{} {}

    /**
     * Move reassign to Some(value)
     */
    constexpr auto operator=(T&& value) -> GenericStorage& {
      inner = std::forward<T>(value);
      return *this;
    }

    /**
     * Copy reassign to Some(value)
     */
    constexpr auto operator=( //
      const T& value
    ) -> GenericStorage& requires std::is_copy_assignable_v<T>
    {
      inner = value;
      return *this;
    }

    /**
     * Reassign to None
     */
    constexpr auto operator=(const None&) -> GenericStorage& {
      inner = crab::None{};
      return *this;
    }

    [[nodiscard]] constexpr auto value() const& -> const T& {
      return std::get<T>(inner);
    }

    [[nodiscard]] constexpr auto value() & -> T& { return std::get<T>(inner); }

    [[nodiscard]] constexpr auto value() && -> T {
      return std::get<T>(std::move(inner));
    }

    [[nodiscard]] constexpr auto in_use() const -> bool {
      return not std::holds_alternative<None>(inner);
    }

  private:

    std::variant<T, None> inner;
  };
#else
  // #error Not using CRAB_OPTION_STD_VARIANT is unsupported currently

  /**
   * Generic tagged union storage for Option<T>
   */
  template<typename T>
  struct GenericStorage {

    /**
     * Initialise to Some(value)
     */
    inline constexpr explicit GenericStorage(T&& value): in_use_flag{true} {
      std::construct_at<T, T&&>(address(), std::forward<T>(value));
    }

    /**
     * Copy initialise to Some(value)
     */
    inline constexpr explicit GenericStorage(const T& value)
      requires std::copy_constructible<T>
        : in_use_flag(true) {
      std::construct_at<T, const T&>(address(), value);
    }

    /**
     * Default initialises to none
     */
    inline constexpr explicit GenericStorage(const crab::None& = crab::none):
        in_use_flag{false} {}

    inline constexpr GenericStorage(const GenericStorage& from):
        in_use_flag{from.in_use_flag} {
      if (in_use()) {
        std::construct_at<T, const T&>(address(), from.value());
      }
    }

    inline constexpr GenericStorage(GenericStorage&& from) noexcept:
        in_use_flag{from.in_use_flag} {
      if (in_use()) {
        std::construct_at<T, T&&>(address(), std::move(*from.address()));
        std::destroy_at(from.address());
        from.in_use_flag = false;
      }
    }

    inline constexpr GenericStorage& operator=(const GenericStorage& from) {
      if (&from == this) {
        return *this;
      }

      if (from.in_use()) {
        operator=(from.value());
      } else {
        operator=(None{});
        return *this;
      }

      return *this;
    }

    inline constexpr GenericStorage& operator=( //
      GenericStorage&& from
    ) noexcept(std::is_nothrow_move_assignable_v<T>) {
      if (not from.in_use()) {
        operator=(None{});
        return *this;
      }

      if (in_use_flag) {
        *address() = std::move(from.value());
      } else {
        std::construct_at<T, T&&>(address(), std::move(from.value()));
        in_use_flag = true;
      }

      std::destroy_at(from.address());
      from.in_use_flag = false;

      return *this;
    }

    inline constexpr ~GenericStorage() {

      if (in_use_flag) {
        std::destroy_at(address());
        in_use_flag = false;
      }
    }

    /**
     * Move reassign to Some(value)
     */
    inline constexpr auto operator=( //
      T&& value
    ) noexcept(std::is_nothrow_move_assignable_v<T>) -> GenericStorage& {
      if (in_use_flag) {
        *address() = std::forward<T>(value);
      } else {
        std::construct_at<T, T&&>(address(), std::forward<T>(value));
        in_use_flag = true;
      }
      return *this;
    }

    /**
     * Copy reassign to Some(value)
     */
    inline constexpr auto operator=( //
      const T& value
    ) -> GenericStorage& requires std::is_copy_assignable_v<T>
    {
      if (in_use_flag) {
        *address() = value;
      } else {
        std::construct_at<T, const T&>(address(), value);
        in_use_flag = true;
      }
      return *this;
    }

    /**
     * Reassign to None
     */
    inline constexpr auto operator=(const None&) -> GenericStorage& {

      if (in_use_flag) {
        std::destroy_at(address());
        in_use_flag = false;
      }

      return *this;
    }

    [[nodiscard]] inline constexpr auto value() const& -> const T& {
      return *address();
    }

    [[nodiscard]] inline constexpr auto value() & -> T& { return *address(); }

    [[nodiscard]] inline constexpr auto value() && -> T {
      T value = std::move(*address());
      std::destroy_at(address());
      in_use_flag = false;
      return value;
    }

    [[nodiscard]] inline constexpr auto in_use() const -> bool {
      return in_use_flag;
    }

  private:

    [[nodiscard]] inline auto address() -> T* {
      return reinterpret_cast<T*>(&bytes[0]);
    }

    [[nodiscard]] inline auto address() const -> const T* {
      return reinterpret_cast<const T*>(&bytes[0]);
    }

    std::array<std::byte, sizeof(T)> bytes alignas(T);
    bool in_use_flag;
  };

#endif

  template<typename T>
  struct RefStorage {
    inline constexpr explicit RefStorage(T& value): inner{&value} {}

    inline constexpr explicit RefStorage(const None& = crab::none):
        inner{nullptr} {}

    inline RefStorage(const RefStorage& from) = default;

    inline RefStorage(RefStorage&& from) noexcept {
      inner = std::exchange(from.inner, nullptr);
    }

    inline RefStorage& operator=(const RefStorage& from) = default;

    inline RefStorage& operator=(RefStorage&& from) = default;

    inline ~RefStorage() = default;

    inline constexpr auto operator=(T& value) -> RefStorage& {
      inner = &value;
      return *this;
    }

    inline constexpr auto operator=(const None&) -> RefStorage& {
      inner = nullptr;
      return *this;
    }

    [[nodiscard]] inline constexpr auto value() const& -> T& { return *inner; }

    [[nodiscard]] inline constexpr auto value() & -> T& { return *inner; }

    [[nodiscard]] inline constexpr auto value() && -> T& {
      return *std::exchange(inner, nullptr);
    }

    [[nodiscard]] inline constexpr auto in_use() const -> bool {
      return inner != nullptr;
    }

  private:

    T* inner;
  };

  template<typename T>
  using Storage = std::conditional_t<
    std::is_reference_v<T>,
    RefStorage<std::remove_reference_t<T>>,
    GenericStorage<T>>;
}

/**
 * Tagged union type between T and unit, alternative to std::optional<T>
 * (or std::variant<T, std::monostate>/std::variant<T>)
 */
template<typename T>
class Option final {
public:

  static_assert(
    not std::same_as<T, crab::None>,
    "Cannot make an option of crab::None, you will need to use some other unit "
    "type (eg. std::monostate, unit, or others)"
  );

  /**
   * Contained inner type
   */
  using Contained = T;

  /**
   * Is the contained type T a reference type (immutable or mutable)
   */
  static constexpr bool is_ref = std::is_reference_v<T>;

  /**
   * Is the contained type T a immutable reference type
   */
  static constexpr bool is_const_ref = is_ref and std::is_const_v<T>;

  /**
   * Is the contained type T a mutable reference type
   */
  static constexpr bool is_mut_ref = is_ref and not std::is_const_v<T>;

  /**
   * @brief Create an option that wraps Some(T)
   */
  constexpr Option(const T& from) noexcept requires(not is_ref)
      : value{from} {}

  /**
   * @brief Create an option that wraps Some(T)
   */
  inline constexpr Option(T&& from) noexcept: value{std::forward<T>(from)} {}

  /**
   * @brief Create an empty option
   */
  inline constexpr Option(crab::None none = {}) noexcept: value{none} {}

  /**
   * Conversion constructor for options of the form Option<T&> to be able to
   * convert implicitly to Option<RefMut<T>> (mainly for backwards
   * compatability)
   */
  [[nodiscard]] inline constexpr operator Option<
    RefMut<std::remove_cvref_t<T>>>() requires(is_mut_ref)
  {
    return map<RefMut<std::remove_cvref_t<T>>>();
  }

  /**
   * Conversion constructor for options of the form Option<const T&>/Option<T&>
   * to be able to convert implicitly to Option<Ref<T>> (mainly for backwards
   * compatability)
   */
  [[nodiscard]] inline constexpr operator Option<Ref<std::remove_cvref_t<T>>>()
    requires(is_ref)
  {
    return map<Ref<std::remove_cvref_t<T>>>();
  }

  /**
   * Conversion constructor for options of the form Option<Ref<T>> to be able to
   * convert implicitly to Option<const T&>
   */
  [[nodiscard]] inline constexpr operator Option<crab::ref_decay<T>>()
    requires(crab::crab_ref<T>)
  {
    return map<const T&>();
  }

  /**
   * Conversion constructor for options of the form Option<Ref<T>> to be able to
   * convert implicitly to Option<T&>
   */
  [[nodiscard]] inline constexpr operator Option<crab::ref_decay<T>>()
    requires(crab::crab_ref_mut<T>)
  {
    return map<T&>();
  }

  /**
   * @brief Reassign option to Some(T),
   * If this option previously contained Some(K), the previous value is
   * discarded and is replaced by Some(T)
   */
  inline constexpr auto operator=(T&& from) -> Option& requires(not is_ref)
  {
    value = std::forward<T>(from);
    return *this;
  }

  /**
   * @brief Reassign option to None,
   * If this option previously contained Some(K), the previous value is
   * discarded and is replaced by Some(T)
   */
  inline constexpr auto operator=(crab::None) -> Option& {
    value = crab::None{};
    return *this;
  }

  /**
   * Move constructor
   */
  inline constexpr Option(Option&& from) /*NOLINT(*explicit*)*/ noexcept:
      value{std::move(from.value)} {}

  /**
   * Implicit move assignment
   */
  constexpr auto operator=(Option&& opt) noexcept -> Option& = default;

  /**
   * Implicit copy constructor
   */
  constexpr Option(const Option&) = default;

  /**
   * Implicit copy assignment
   */
  constexpr auto operator=(const Option&) -> Option& = default;

  /**
   * Implicit destructor
   */
  constexpr ~Option() = default;

  /**
   * @brief Whether this option has a contained value or not (None)
   */
  [[nodiscard]] inline constexpr explicit operator bool() const {
    return is_some();
  }

  /**
   * @brief Whether this option contains a value
   */
  [[nodiscard]] inline constexpr auto is_some() const -> bool {
    return value.in_use();
  }

  /**
   * @brief Whether this option does not contain a value
   */
  [[nodiscard]] inline constexpr auto is_none() const -> bool {
    return not value.in_use();
  }

  /**
   * @brief Converts a 'const Option<T>&' into a Option<const T&>, to give
   * optional access to the actual referenced value inside.
   *
   * This function returns Option<const T&>
   */
  [[nodiscard]]
  inline constexpr auto as_ref() const& requires(not is_ref)
  {
    if (is_none()) {
      return Option<const T&>{};
    }
    return Option<const T&>{get_unchecked()};
  }

  /**
   * @brief Converts a 'Option<T>&' into a
   * Option<T&>, to give optional access to the actual referenced value
   * inside.
   *
   * This function returns Option<T&>
   */
  [[nodiscard]] inline constexpr auto as_mut() & requires(not is_ref)
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
  [[nodiscard]] inline constexpr auto take_or_default() && -> T
    requires std::constructible_from<T>
  {
    return std::move(*this).take_or([] { return T{}; });
  }

  /**
   * @brief Takes the contained value (like Option<T>::unwrap()) if
   * exists, else returns a default value
   * @param default_value
   */
  [[nodiscard]] inline constexpr auto take_or(T&& default_value) && -> T {
    return is_some() ? T{std::move(*this).unwrap()}
                     : std::forward<Contained>(default_value);
  }

  /**
   * @brief Takes the contained value (like Option<T>::unwrap()) if
   * exists, else uses 'F' to compute & create a default value
   * @param default_generator Function to create the default value
   */
  template<std::invocable F>
  [[nodiscard]] inline constexpr auto take_or(F&& default_generator) && -> T {
    return is_some() ? T{std::move(*this).unwrap()}
                     : T{std::invoke(default_generator)};
  }

  /**
   * @brief Is this option has a value, return a copy of that value. if
   * this opton was none then this returns a default constructed T
   */
  [[nodiscard]] constexpr inline auto get_or_default() const -> T
    requires std::constructible_from<T> and std::copyable<T>
  {
    return get_or([] { return T{}; });
  }

  /**
   * @brief Gets the contained value if exists, else returns a default value
   * @param default_value
   */
  [[nodiscard]] inline constexpr auto get_or(T default_value) const -> T
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
  [[nodiscard]] inline constexpr auto get_or(F default_generator) const -> T
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
  [[nodiscard]] inline constexpr auto unwrap(
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
  [[nodiscard]] inline constexpr auto get_unchecked(
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
  [[nodiscard]] inline constexpr auto get_unchecked(
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
  [[nodiscard]] inline constexpr auto ok_or(E&& error) const -> Result<T, E>
    requires std::copy_constructible<T>
  {
    return is_some() ? Result<T, E>{crab::Ok<T>{get_unchecked()}}
                     : Result<T, E>{crab::Err<E>{std::forward<E>(error)}};
  }

  /**
   * @brief Creates a Result<T, E> from this given option, where "None" is
   * expanded to some error given.
   * @tparam E Error Type
   * @param error_generator Function to generate an error to replace "None".
   */
  template<typename E, std::invocable F>
  [[nodiscard]]
  inline constexpr auto ok_or(F&& error_generator) const -> Result<T, E>
    requires std::copy_constructible<T>
         and std::convertible_to<std::invoke_result_t<F>, E>
  {
    return is_some() ? Result<T, E>{crab::Ok<T>{get_unchecked()}}
                     : Result<T, E>{crab::Err<E>{std::invoke(error_generator)}};
  }

  /**
   * @brief Converts this Result<T, E> from this given option, where "None" is
   * expanded to some error given. This will invalidate the Option<T> after
   * call, just like every other Option::take_* function.
   * @tparam E Error Type
   * @param error Error to replace an instance of None
   */
  template<typename E>
  [[nodiscard]] inline constexpr auto take_ok_or(E error) && -> Result<T, E> {
    return is_some() ? Result<T, E>{crab::Ok<T>{std::move(*this).unwrap()}}
                     : Result<T, E>(crab::Err<E>{std::move(error)});
  }

  /**
   * @brief Creates a Result<T, E> from this given option, where "None" is
   * expanded to some error given.
   * @tparam E Error Type
   * @param error_generator Function to generate an error to replace "None".
   */
  template<typename E, std::invocable F>
  [[nodiscard]]
  inline constexpr auto take_ok_or(F&& error_generator) && -> Result<T, E> {
    return is_some() ? Result<T, E>{crab::Ok<T>{std::move(*this).unwrap()}}
                     : Result<T, E>(crab::Err<E>{std::invoke(error_generator)});
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
  [[nodiscard]] inline constexpr auto map(F&& mapper) && {
    using Returned = Option<std::invoke_result_t<F, T>>;

    return is_some() ? Returned{std::invoke(mapper, std::move(*this).unwrap())}
                     : Returned{};
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
  [[nodiscard]] inline constexpr auto map(F&& mapper) const& {
    static_assert(
      std::copy_constructible<T>,
      "'Option<T>::map<F> const&' can only be done if T is copy constructible, "
      "possible solution is moving the underlying option to use "
      "Option<T>::map<F> &&"
    );
    return copied().map(std::forward<F>(mapper));
  }

  template<typename Into>
  [[nodiscard]] inline constexpr auto map() const& {
    static_assert(
      std::convertible_to<T, Into> and std::copy_constructible<T>,
      "'Option<T>::map<Into>() const&' can only be done if T is convertible to "
      "Into and T is copy constructible"
    );
    return copied().template map<Into>();
  }

  template<typename Into>
  [[nodiscard]] inline constexpr auto map() && {
    static_assert(
      std::convertible_to<T, Into>,
      "'Option<T>::map<Into>()' can only be done if T is convertible to Into"
    );
    return std::move(*this).map([](T&& value) -> Into {
      return static_cast<Into>(std::forward<Contained>(value));
    });
  }

  /**
   * @brief Shorthand for calling .map(...).flatten()
   */
  template<std::invocable<T> F>
  [[nodiscard]] inline constexpr auto flat_map(F mapper) && {
    using Returned = std::invoke_result_t<F, T>;
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
  [[nodiscard]] inline constexpr auto flat_map(F mapper) const& {
    return copied().flat_map(std::forward<F>(mapper));
  }

  /**
   * @brief Equivalent of flat_map but for the 'None' type
   */
  template<std::invocable F>
  [[nodiscard]] inline constexpr auto or_else(F mapper) && {
    using Returned = std::invoke_result_t<F>;

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
  [[nodiscard]] inline constexpr auto or_else(F mapper) const& {
    return copied().or_else(std::forward<F>(mapper));
  }

  /**
   * @brief If this option is of some type Option<Option<T>>, this will flatten
   * it to a single Option<T>
   */
  [[nodiscard]] inline constexpr auto flatten() && -> T
    requires crab::option_type<T>
  {
    return std::move(*this).flat_map([](T value) { return value; });
  }

  /**
   * @brief If this option is of some type Option<Option<T>>, this will flatten
   * it to a single Option<T>
   */
  [[nodiscard]] inline constexpr auto flatten() const& -> T
    requires crab::option_type<T> and std::copy_constructible<T>
  {
    return copied().flatten();
  }

  /**
   * @brief Copies this option and returns, use this before map if you do not
   * want to consume the option.
   */
  [[nodiscard]] inline constexpr auto copied() const -> Option
    requires std::copy_constructible<T>
  {
    if (is_some()) {
      return Option<T>{get_unchecked()};
    }

    return crab::None{};
  }

  /**
   * @brief Consumes this option, if this is some and passes the predicate it
   * will return Some, else None
   */
  template<std::predicate<const T&> F>
  [[nodiscard]] inline constexpr auto filter(F predicate) && -> Option {
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
  [[nodiscard]] inline constexpr auto filter(F predicate) const& -> Option<T> {
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
  [[nodiscard]] inline constexpr auto filter(F predicate) & -> Option<T> {
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
  [[nodiscard]] inline constexpr auto zip( //
    Option<Vals>... other
  ) && -> Option<Tuple<T, Vals...>> {
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
  /// Boolean Operations
  ///

  /**
   * Boolean 'and' operation on two options, this will return none if either of
   * the two options are none, and if both are some this will return the
   * contained value of the second
   */
  template<typename S>
  [[nodiscard]]
  inline constexpr auto operator and(Option<S> other) && -> Option<S> {
    if (is_none()) {
      return crab::none;
    }

    return other;
  }

  /**
   * Boolean 'and' operation on two options, this will return none if either of
   * the two options are none, and if both are some this will return the
   * contained value of the second
   */
  template<typename S>
  [[nodiscard]]
  inline constexpr auto operator and(Option<S> other) const& -> Option<S>
    requires std::copy_constructible<T>
  {
    return copied() and std::move(other);
  }

  /**
   * Boolean or operation, if this option is some it will return that, if the
   * other option is some it will return that, else this returns none
   */
  [[nodiscard]] inline constexpr auto operator or(Option other) && -> Option {
    if (is_none()) {
      return other;
    }

    return std::move(*this);
  }

  /**
   * Boolean or operation, if this option is some it will return that, if the
   * other option is some it will return that, else this returns none
   */
  [[nodiscard]]
  inline constexpr auto operator or(Option other) const& -> Option
    requires std::copy_constructible<T>
  {
    return copied() or std::move(other);
  }

  /**
   * The same as or but if both options are some then this will be none
   */
  [[nodiscard]] inline constexpr auto operator xor(Option other) && -> Option {
    if (is_none()) {
      return other;
    }

    if (other.is_some()) {
      return crab::none;
    }

    return std::move(*this);
  }

  /**
   * The same as or but if both options are some then this will be none
   */
  [[nodiscard]]
  inline constexpr auto operator xor(Option other) const& -> Option
    requires std::copy_constructible<T>
  {
    return copied() xor std::move(other);
  }

  ///
  /// Ordering Overloads
  ///

  template<typename S>
  [[nodiscard]]
  inline constexpr auto operator==(const Option<S>& other) const -> bool {
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
  inline constexpr auto operator!=(const Option<S>& other) const -> bool {
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
  inline constexpr auto operator>(const Option<S>& other) const -> bool {
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
  inline constexpr auto operator<(const Option<S>& other) const -> bool {
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
  inline constexpr auto operator>=(const Option<S>& other) const -> bool {
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
  inline constexpr auto operator<=(const Option<S>& other) const -> bool {
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
  [[nodiscard]] inline constexpr auto operator<=>( //
    const Option<S>& other
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
  [[nodiscard]] inline constexpr auto operator==(const crab::None&) const
    -> bool {
    return is_none();
  }

  /**
   * @brief Option<T> != None only if the former is some
   */
  [[nodiscard]] inline constexpr auto operator!=(const crab::None&) const
    -> bool {
    return is_some();
  }

  /**
   * @brief Option<T> > None only if the former is some
   */
  [[nodiscard]] inline constexpr auto operator>(const crab::None&) const
    -> bool {
    return is_some();
  }

  /**
   * @brief Option<T> >= None is always true
   */
  [[nodiscard]] inline constexpr auto operator>=(const crab::None&) const
    -> bool {
    return true;
  }

  /**
   * @brief Option<T> < None is never true
   */
  [[nodiscard]] inline constexpr auto operator<(const crab::None&) const
    -> bool {
    return false;
  }

  /**
   * @brief Option<T> <= None is never true
   */
  [[nodiscard]] inline constexpr auto operator<=(const crab::None&) const
    -> bool {
    return is_none();
  }

private:

  crab::option::helper::Storage<T> value;
};

template<typename T>
inline constexpr auto operator<<(std::ostream& os, const Option<T>& opt)
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

template<typename T>
struct std::hash<Option<T>> /*NOLINT*/ {
  [[nodiscard]] inline constexpr auto operator()( //
    const Option<T>& opt
  ) const -> crab::hash_code {
    if (opt.is_none()) {
      return 0;
    }

    return crab::hash_together<usize, T>(1, opt.get_unchecked());
  }
};

namespace crab {

  /**
   * @brief Creates an Option<T> from some value T
   */
  template<typename T>
  [[nodiscard]] inline constexpr auto some(std::type_identity_t<T> from) {
    return Option<std::remove_cvref_t<T>>{std::move(from)};
  }

  /**
   * @brief Creates an Option<T> from some value T
   */
  [[nodiscard]] inline constexpr auto some(auto from) {
    return Option<std::remove_cvref_t<decltype(from)>>{std::move(from)};
  }

  /**
   * @brief Maps a boolean to an option if it is true
   */
  template<std::invocable F>
  [[nodiscard]] inline constexpr auto then(const bool cond, F&& func) {
    if (not cond) {
      return Option<std::invoke_result_t<F>>{};
    }
    return Option<std::invoke_result_t<F>>{std::invoke(func)};
  }

  /**
   * @brief Maps a boolean to an option if it is false
   */
  template<std::invocable F>
  [[nodiscard]] inline constexpr auto unless(const bool cond, F&& func)
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
  [[nodiscard]] inline constexpr auto unwrap(Option<T>&& from) -> T {
    return std::forward<T>(from).unwrap();
  }

  namespace option {
    struct fallible final {
      template<typename... T>
      [[nodiscard]] inline constexpr auto operator()(Tuple<T...> tuple) const {
        return Option<Tuple<T...>>{std::forward<Tuple<T...>>(tuple)};
      }

      template<typename PrevResults, std::invocable F, typename... Rest>
      requires option_type<std::invoke_result_t<F>>
      [[nodiscard]] inline constexpr auto operator()(
        PrevResults tuple /* Tuple<T...>*/,
        F&& function,
        Rest&&... other_functions
      ) const {
        return std::invoke(function).flat_map([&]<typename R>(R&& result) {
          return operator()(
            std::tuple_cat(std::move(tuple), Tuple<R>(std::forward<R>(result))),
            std::forward<Rest>(other_functions)...
          );
        });
      }

      template<typename PrevResults, std::invocable F, typename... Rest>
      requires(not option_type<std::invoke_result_t<F>>)
      [[nodiscard]] inline constexpr auto operator()(
        PrevResults tuple /* Tuple<T...>*/,
        F&& function,
        Rest&&... other_functions
      ) const {
        return operator()(
          std::tuple_cat(
            std::move(tuple),
            Tuple<std::invoke_result_t<F>>(std::invoke(function))
          ),
          std::forward<Rest>(other_functions)...
        );
      }

      template<typename PrevResults, typename V, typename... Rest>
      requires(not std::invocable<V> and not option_type<V>)
      [[nodiscard]] inline constexpr auto operator()(
        PrevResults tuple /* Tuple<T...>*/,
        V&& value,
        Rest&&... other_functions
      ) const {
        return operator()(
          std::tuple_cat(std::move(tuple), Tuple<V>{std::forward<V>(value)}),
          std::forward<Rest>(other_functions)...
        );
      }

      template<typename PrevResults, typename V, typename... Rest>
      requires(not std::invocable<V>)
      [[nodiscard]] inline constexpr auto operator()(
        PrevResults tuple, /* Tuple<T...>*/
        Option<V> value,
        Rest&&... other_functions
      ) const {
        return std::move(value).flat_map([&](V&& result) {
          return operator()(
            std::tuple_cat(std::move(tuple), Tuple<V>(std::forward<V>(result))),
            std::forward<Rest>(other_functions)...
          );
        });
      }
    };
  } // namespace option

  template<typename... F>
  [[nodiscard]] inline constexpr auto fallible(F&&... fallible) {
    return option::fallible{}(Tuple<>{}, std::forward<F>(fallible)...);
  }
} // namespace crab

// NOLINTEND(*explicit*)

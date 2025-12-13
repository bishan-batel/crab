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
#include <cstddef>
#include <functional>
#include <iostream>
#include <type_traits>
#include <utility>

#include <crab/preamble.hpp>
#include <crab/type_traits.hpp>
#include <crab/debug.hpp>
#include <crab/hash.hpp>

namespace crab {
  /**
   * 0-sized* struct to give into Option<T> to create an empty Option
   */
  struct None final {
    /**
     * Equality comparison for none, None is a unit type therefore this will
     * always return true.
     */
    CRAB_PURE_INLINE_CONSTEXPR auto operator==(const None&) const -> bool {
      return true;
    }
  };

  /**
   * 'None' value type for use with Option<T>
   */
  inline static CRAB_CONSTEXPR None none{};

} // namespace crab

/**
 * Helper utilities for implementation of Option<T>, namely the
 * internal storage method
 */
namespace crab::option {

  // #error Not using CRAB_OPTION_STD_VARIANT is unsupported currently

  /**
   * Generic tagged union storage for Option<T>
   */
  template<typename T>
  struct GenericStorage {

    /**
     * Initialise to Some(value)
     */
    CRAB_INLINE_CONSTEXPR explicit GenericStorage(T&& value): in_use_flag{true} {
      std::construct_at<T, T&&>(address(), std::forward<T>(value));
    }

    /**
     * Copy initialise to Some(value)
     */
    CRAB_INLINE_CONSTEXPR explicit GenericStorage(const T& value) requires crab::ty::copy_constructible<T>
        : in_use_flag(true) {
      std::construct_at<T, const T&>(address(), value);
    }

    /**
     * Default initialises to none
     */
    CRAB_INLINE_CONSTEXPR explicit GenericStorage(const crab::None& = crab::none): in_use_flag{false} {}

    CRAB_CONSTEXPR GenericStorage(const GenericStorage& from) requires crab::ty::copy_constructible<T>
        : in_use_flag{from.in_use_flag} {
      if (in_use()) {
        std::construct_at<T, const T&>(address(), from.value());
      }
    }

    CRAB_CONSTEXPR GenericStorage(GenericStorage&& from) noexcept: in_use_flag{from.in_use_flag} {
      if (in_use()) {
        std::construct_at<T, T&&>(address(), std::move(from.value()));
        std::destroy_at(from.address());
        from.in_use_flag = false;
      }
    }

    CRAB_CONSTEXPR GenericStorage& operator=(const GenericStorage& from) {
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

    CRAB_CONSTEXPR GenericStorage& operator=(GenericStorage&& from) noexcept(std::is_nothrow_move_assignable_v<T>) {
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

    CRAB_CONSTEXPR ~GenericStorage() {

      if (in_use_flag) {
        std::destroy_at(address());
        in_use_flag = false;
      }
    }

    /**
     * Move reassign to Some(value)
     */
    CRAB_CONSTEXPR auto operator=(T&& value) noexcept(std::is_nothrow_move_assignable_v<T>) -> GenericStorage& {
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
    CRAB_CONSTEXPR auto operator=(const T& from) -> GenericStorage& requires crab::ty::copy_assignable<T>
    {
      if (in_use_flag) {
        value() = from;
      } else {
        std::construct_at<T, const T&>(address(), from);
        in_use_flag = true;
      }
      return *this;
    }

    /**
     * Reassign to None
     */
    CRAB_CONSTEXPR auto operator=(const None&) -> GenericStorage& {

      if (in_use_flag) {
        std::destroy_at(address());
        in_use_flag = false;
      }

      return *this;
    }

    CRAB_PURE_INLINE_CONSTEXPR auto value() const& -> const T& {
      return reinterpret_cast<const T&>(bytes);
    }

    CRAB_PURE_INLINE_CONSTEXPR auto value() & -> T& {
      return reinterpret_cast<T&>(bytes);
    }

    CRAB_PURE_INLINE_CONSTEXPR auto value() && -> T {
#if CRAB_GCC_VERSION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
      T moved{std::move(reinterpret_cast<T&>(bytes))};

#if CRAB_GCC_VERSION
#pragma GCC diagnostic pop
#endif
      std::destroy_at<T>(address());
      in_use_flag = false;
      return moved;
    }

    CRAB_PURE_INLINE_CONSTEXPR auto in_use() const -> bool {
      return in_use_flag;
    }

  private:

    CRAB_PURE_INLINE_CONSTEXPR CRAB_RETURNS_NONNULL auto address() -> T* {
      return reinterpret_cast<T*>(&bytes);
    }

    CRAB_PURE_INLINE_CONSTEXPR CRAB_RETURNS_NONNULL auto address() const -> const T* {
      return reinterpret_cast<const T*>(&bytes);
    }

    alignas(T) SizedArray<std::byte, sizeof(T)> bytes{};

    bool in_use_flag;
  };

  template<typename T>
  struct RefStorage {
    CRAB_INLINE_CONSTEXPR explicit RefStorage(T& value): inner{&value} {}

    CRAB_INLINE_CONSTEXPR explicit RefStorage(const None& = crab::none): inner{nullptr} {}

    CRAB_INLINE_CONSTEXPR RefStorage(const RefStorage& from) = default;

    CRAB_INLINE_CONSTEXPR RefStorage(RefStorage&& from) noexcept {
      inner = std::exchange(from.inner, nullptr);
    }

    CRAB_INLINE_CONSTEXPR RefStorage& operator=(const RefStorage& from) = default;

    CRAB_INLINE_CONSTEXPR RefStorage& operator=(RefStorage&& from) = default;

    CRAB_INLINE_CONSTEXPR ~RefStorage() = default;

    CRAB_INLINE_CONSTEXPR auto operator=(T& value) -> RefStorage& {
      inner = &value;
      return *this;
    }

    CRAB_INLINE_CONSTEXPR auto operator=(const None&) -> RefStorage& {
      inner = nullptr;
      return *this;
    }

    CRAB_PURE_INLINE_CONSTEXPR auto value() const& -> T& {
      return *inner;
    }

    CRAB_PURE_INLINE_CONSTEXPR auto value() & -> T& {
      return *inner;
    }

    CRAB_PURE_INLINE_CONSTEXPR auto value() && -> T& {
      return *std::exchange(inner, nullptr);
    }

    CRAB_PURE_INLINE_CONSTEXPR auto in_use() const -> bool {
      return inner != nullptr;
    }

  private:

    T* inner;
  };

  /**
   * Storage selector for Option<T>, this type's public alias 'type' determines the structure
   * used as the internal storage for Option<T>
   */
  template<typename T>
  struct Storage final {
    using type = GenericStorage<T>;
  };

  template<ty::is_reference T>
  struct Storage<T> final {
    using type = RefStorage<ty::remove_reference<T>>;
  };
}

/**
 * Tagged union type between T and unit, alternative to std::optional<T>
 * (or std::variant<T, std::monostate>/std::variant<T>)
 *
 * This type is 'well-behaved'
 */
template<typename T>
class Option final {
public:

  static_assert(
    not crab::ty::same_as<T, crab::None>,
    "Cannot make an option of crab::None, you will need to use some other unit "
    "type (eg. std::monostate, unit, or others)"
  );

  /**
   * Contained inner type
   */
  using Contained = T;

  /**
   * Storage type for this data
   */
  using Storage = crab::option::Storage<T>::type;

  /**
   * Is the contained type T a reference type (immutable or mutable)
   */
  inline static CRAB_CONSTEXPR bool is_ref = crab::ty::is_reference<T>;

  /**
   * Is the contained type T a immutable reference type
   */
  inline static CRAB_CONSTEXPR bool is_const_ref = is_ref and crab::ty::is_const<T>;

  /**
   * Is the contained type T a mutable reference type
   */
  inline static CRAB_CONSTEXPR bool is_mut_ref = is_ref and crab::ty::non_const<T>;

  /**
   * Create an option that wraps Some(T)
   */
  CRAB_INLINE_CONSTEXPR Option(const T& from) noexcept(std::is_nothrow_copy_constructible_v<T>) requires(not is_ref)
      : value{from} {}

  /**
   * Create an option that wraps Some(T)
   */
  CRAB_INLINE_CONSTEXPR Option(T&& from) noexcept(std::is_nothrow_move_constructible_v<T>):
      value{std::forward<T>(from)} {}

  /**
   * Create an empty option
   */
  CRAB_INLINE_CONSTEXPR Option(crab::None none = {}) noexcept: value{none} {}

  /**
   * Conversion constructor for options of the form Option<T&> to be able to
   * convert implicitly to Option<RefMut<T>> (mainly for backwards
   * compatability)
   */
  CRAB_PURE_INLINE_CONSTEXPR operator Option<RefMut<std::remove_cvref_t<T>>>() requires(is_mut_ref)
  {
    return map<RefMut<std::remove_cvref_t<T>>>();
  }

  /**
   * Conversion constructor for options of the form Option<const T&>/Option<T&>
   * to be able to convert implicitly to Option<Ref<T>> (mainly for backwards
   * compatability)
   */
  CRAB_PURE_INLINE_CONSTEXPR operator Option<Ref<std::remove_cvref_t<T>>>() requires(is_ref)
  {
    return map<Ref<std::remove_cvref_t<T>>>();
  }

  /**
   * Conversion constructor for options of the form Option<Ref<T>> to be able to
   * convert implicitly to Option<const T&>
   */
  CRAB_PURE_INLINE_CONSTEXPR operator Option<crab::ty::crab_ref_decay<T>>() requires(crab::ty::crab_ref<T>)
  {
    return map<const T&>();
  }

  /**
   * Conversion constructor for options of the form Option<Ref<T>> to be able to
   * convert implicitly to Option<T&>
   */
  CRAB_PURE_INLINE_CONSTEXPR operator Option<crab::ty::crab_ref_decay<T>>() requires(crab::ty::crab_ref_mut<T>)
  {
    return map<T&>();
  }

  /**
   * Reassign option to Some(T),
   * If this option previously contained Some(K), the previous value is
   * discarded and is replaced by Some(T)
   */
  CRAB_INLINE_CONSTEXPR auto operator=(T&& from) -> Option& requires(not is_ref)
  {
    value = std::forward<T>(from);
    return *this;
  }

  /**
   * Reassign option to None,
   * If this option previously contained Some(K), the previous value is
   * discarded and is replaced by Some(T)
   */
  CRAB_INLINE_CONSTEXPR auto operator=(crab::None) -> Option& {
    value = crab::None{};
    return *this;
  }

  /**
   * Move constructor
   */
  CRAB_INLINE_CONSTEXPR Option(Option&& from) = default;

  /**
   * Implicit move assignment
   */
  CRAB_INLINE_CONSTEXPR auto operator=(Option&& opt) noexcept -> Option& = default;

  /**
   * Implicit copy constructor
   */
  CRAB_INLINE_CONSTEXPR Option(const Option&) = default;

  /**
   * Implicit copy assignment
   */
  CRAB_INLINE_CONSTEXPR auto operator=(const Option&) -> Option& = default;

  /**
   * Implicit destructor
   */
  CRAB_INLINE_CONSTEXPR ~Option() = default;

  /**
   * Whether this option has a contained value or not (None)
   */
  CRAB_PURE_INLINE_CONSTEXPR explicit operator bool() const {
    return is_some();
  }

  /**
   * Whether this option contains a value
   */
  CRAB_PURE_INLINE_CONSTEXPR auto is_some() const -> bool {
    return value.in_use();
  }

  /**
   * Whether this option does not contain a value
   */
  CRAB_PURE_INLINE_CONSTEXPR auto is_none() const -> bool {
    return not value.in_use();
  }

  /**
   * Converts a 'const Option<T>&' into a Option<const T&>, to give
   * optional access to the actual referenced value inside.
   *
   * This function returns Option<const T&>
   */
  CRAB_PURE_INLINE_CONSTEXPR auto as_ref() const& -> Option<const T&> requires(not is_ref)
  {
    if (is_none()) {
      return crab::none;
    }

    return Option<const T&>{get_unchecked()};
  }

  /**
   * Converts a 'Option<T>&' into a
   * Option<T&>, to give optional access to the actual referenced value
   * inside.
   *
   * This function returns Option<T&>
   */
  CRAB_PURE_INLINE_CONSTEXPR auto as_mut() & -> Option<T&> requires(not is_ref)
  {
    if (is_none()) {
      return crab::none;
    }

    return Option<T&>{get_unchecked()};
  }

  /**
   * Consumes inner value and returns it, if this option was none this
   * will instead return a new default constructed T
   */
  CRAB_PURE_INLINE_CONSTEXPR auto take_or_default() && -> T requires crab::ty::default_constructible<T>
  {
    static_assert(crab::ty::default_constructible<T>, "take_or_default requires that T is default constructible");

    return std::move(*this).take_or([]() { return T{}; });
  }

  /**
   * Takes the contained value (like Option<T>::unwrap()) if
   * exists, else returns a default value
   * @param default_value
   */
  CRAB_PURE_INLINE_CONSTEXPR auto take_or(T default_value) && -> T {
    return is_some() ? std::move(*this).unwrap() : std::forward<T>(default_value);
  }

  /**
   * Takes the contained value (like Option<T>::unwrap()) if
   * exists, else uses 'F' to compute & create a default value
   * @param default_generator Function to create the default value
   */
  template<crab::ty::provider<T> F>
  CRAB_PURE_INLINE_CONSTEXPR auto take_or(F&& default_generator) && -> T {
    return is_some() ? T{std::move(*this).unwrap()} : std::invoke(default_generator);
  }

  /**
   * Is this option has a value, return a copy of that value. if
   * this opton was none then this returns a default constructed T
   */
  CRAB_PURE_INLINE_CONSTEXPR auto get_or_default() const -> T requires crab::ty::copy_constructible<T>
  {
    return copied().take_or_default();
  }

  /**
   * Gets the contained value if exists, else returns a default value
   * @param default_value
   */
  CRAB_PURE_INLINE_CONSTEXPR auto get_or(T default_value) const -> T requires crab::ty::copy_constructible<T>
  {
    return copied().take_or(std::forward<T>(default_value));
  }

  /**
   * Gets the contained value if exists, else computes a default value
   * with 'F' and returns
   * @param default_generator Function to create the default value
   */
  template<crab::ty::provider<T> F>
  CRAB_PURE_INLINE_CONSTEXPR auto get_or(F&& default_generator) const -> T requires crab::ty::copy_constructible<T>
  {
    static_assert(std::is_invocable_r_v<T, F>, "A function with Option<T>::get_or must return T");
    return copied().take_or(std::forward<F>(default_generator));
  }

  /**
   * Takes value out of the option and returns it, will error if option
   * is none, After this, the value has been 'taken out' of this option, after
   * this method is called this option is 'None'
   */
  CRAB_PURE_INLINE_CONSTEXPR auto unwrap(SourceLocation loc = SourceLocation::current()) && -> T {
    debug_assert_transparent(is_some(), loc, "Cannot unwrap a none option");
    return std::move(value).value();
  }

  /**
   * Returns a mutable reference to the contained Some value inside, if
   * this option is none this will panic & crash.
   */
  CRAB_PURE_INLINE_CONSTEXPR auto get_unchecked(SourceLocation loc = SourceLocation::current()) -> T& {
    debug_assert_transparent(is_some(), loc, "Cannot get_unchecked a none option");

    return value.value();
  }

  /**
   * Returns a const reference to the contained Some value inside, if
   * this option is none this will panic & crash.
   */
  CRAB_PURE_INLINE_CONSTEXPR auto get_unchecked(SourceLocation loc = SourceLocation::current()) const -> const T& {
    debug_assert_transparent(is_some(), loc, "Cannot get_unchecked a none option");
    return value.value();
  }

  /**
   * Creates a Result<T, E> from this given option, where "None" is
   * expanded to some error given.
   * @tparam E Error Type
   * @param error Error to replace an instance of None
   */
  template<typename E>
  CRAB_PURE_INLINE_CONSTEXPR auto ok_or(E&& error) const -> Result<T, E> requires crab::ty::copy_constructible<T>
  {
    if (is_some()) {
      return crab::Ok<T>{get_unchecked()};
    }

    return crab::Err<E>{std::forward<E>(error)};
  }

  /**
   * Creates a Result<T, E> from this given option, where "None" is
   * expanded to some error given.
   * @tparam E Error Type
   * @param error_generator Function to generate an error to replace "None".
   */
  template<typename E, crab::ty::provider<E> F>
  CRAB_PURE_INLINE_CONSTEXPR auto ok_or(F&& error_generator) const
    -> Result<T, E> requires crab::ty::copy_constructible<T>
  {
    if (is_some()) {
      return crab::Ok<T>{get_unchecked()};
    }

    return crab::Err<E>{std::invoke(error_generator)};
  }

  /**
   * Converts this Result<T, E> from this given option, where "None" is
   * expanded to some error given. This will invalidate the Option<T> after
   * call, just like every other Option::take_* function.
   * @tparam E Error Type
   * @param error Error to replace an instance of None
   */
  template<typename E>
  CRAB_PURE_INLINE_CONSTEXPR auto take_ok_or(E error) && -> Result<T, E> {
    return is_some() ? Result<T, E>{crab::Ok<T>{std::move(*this).unwrap()}}
                     : Result<T, E>(crab::Err<E>{std::move(error)});
  }

  /**
   * Creates a Result<T, E> from this given option, where "None" is
   * expanded to some error given.
   * @tparam E Error Type
   * @param error_generator Function to generate an error to replace "None".
   */
  template<typename E, crab::ty::provider<E> F>
  CRAB_PURE_INLINE_CONSTEXPR auto take_ok_or(F&& error_generator) && -> Result<T, E> {
    return is_some() ? Result<T, E>{crab::Ok<T>{std::move(*this).unwrap()}}
                     : Result<T, E>(crab::Err<E>{std::invoke(error_generator)});
  }

  /**
   * Transform function to an Option<T> -> Option<K>, if this option is
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
  template<crab::ty::mapper<T> F>
  CRAB_PURE_INLINE_CONSTEXPR auto map(F&& mapper) && {
    using Returned = Option<crab::ty::mapper_codomain<F, T>>;

    return is_some() ? Returned{std::invoke(mapper, std::move(*this).unwrap())} : Returned{};
  }

  /**
   * Transform function to an Option<T> -> Option<K>, if this option is
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
   * assert(Option<i32>{420}
   *  .map([](i32 x) { return std::tostring(x); }) // returns a Option<String>
   *  .unwrap() == "420"
   * );
   */
  template<crab::ty::mapper<T> F>
  requires crab::ty::copy_constructible<T>
  CRAB_PURE_INLINE_CONSTEXPR auto map(F&& mapper) const& {
    return copied().map(std::forward<F>(mapper));
  }

  template<typename Into>
  CRAB_PURE_INLINE_CONSTEXPR auto map() && -> Option<Into> {
    static_assert(
      crab::ty::convertible<T, Into>,
      "'Option<T>::map<Into>()' can only be done if T is convertible to Into"
    );

    return std::move(*this).map([](T&& value) { return static_cast<Into>(std::forward<T>(value)); });
  }

  template<typename Into>
  requires crab::ty::copy_constructible<T>
  CRAB_PURE_INLINE_CONSTEXPR auto map() const& -> Option<Into> {
    return copied().template map<Into>();
  }

  /**
   * Shorthand for calling .map(...).flatten()
   */
  template<crab::ty::mapper<T> F>
  CRAB_PURE_INLINE_CONSTEXPR auto flat_map(F&& mapper) && {
    using Returned = crab::ty::mapper_codomain<F, T>;
    static_assert(crab::option_type<Returned>, "The function passed to flat_map must return an Option");

    if (is_some()) {
      return Returned{std::invoke(mapper, std::move(*this).unwrap())};
    }

    return Returned{crab::None{}};
  }

  /**
   * Shorthand for calling .map(...).flatten()
   */
  template<crab::ty::mapper<T> F>
  requires crab::ty::copy_constructible<T>
  CRAB_PURE_INLINE_CONSTEXPR auto flat_map(F&& mapper) const& {
    return copied().flat_map(std::forward<F>(mapper));
  }

  /**
   * Equivalent of flat_map but for the 'None' type
   */
  template<crab::ty::provider F>
  CRAB_PURE_INLINE_CONSTEXPR auto or_else(F&& mapper) && {
    using Returned = crab::ty::functor_result<F>;

    static_assert(crab::option_type<Returned>, "The function passed to or_else must return an Option");

    if (is_some()) {
      return Returned{std::move(*this).unwrap()};
    }

    return Returned{std::invoke(mapper)};
  }

  /**
   * Equivalent of flat_map but for the 'None' type
   */
  template<crab::ty::provider F>
  requires crab::ty::copy_constructible<T>
  CRAB_PURE_INLINE_CONSTEXPR auto or_else(F&& mapper) const& {
    return copied().or_else(std::forward<F>(mapper));
  }

  /**
   * If this option is of some type Option<Option<T>>, this will flatten
   * it to a single Option<T>
   */
  CRAB_PURE_INLINE_CONSTEXPR auto flatten() && -> T requires crab::option_type<T>
  {
    if (is_none()) {
      return crab::none;
    }

    return std::move(*this).unwrap();
  }

  /**
   * If this option is of some type Option<Option<T>>, this will flatten
   * it to a single Option<T>
   */
  CRAB_PURE_INLINE_CONSTEXPR auto flatten() const& -> T
    requires crab::option_type<T> and crab::ty::copy_constructible<T>

  {
    return copied().flatten();
  }

  /**
   * Copies this option and returns, use this before map if you do not
   * want to consume the option.
   */
  CRAB_PURE_INLINE_CONSTEXPR auto copied() const -> Option requires crab::ty::copy_constructible<T>
  {
    static_assert(
      crab::ty::copy_constructible<T>,
      "Cannot call copied() on an option with a non copy-cosntructible type"
    );

    if (is_some()) {
      return Option<T>{get_unchecked()};
    }

    return crab::None{};
  }

  /**
   * Consumes this option, if this is some and passes the predicate it
   * will return Some, else None
   */
  template<crab::ty::predicate<const T&> F>
  CRAB_PURE_INLINE_CONSTEXPR auto filter(F&& predicate) && -> Option {
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
   * Copys this option, if this is some and passes the predicate it
   * will return Some, else None
   */
  template<crab::ty::predicate<const T&> F>
  requires crab::ty::copy_constructible<T>
  CRAB_PURE_INLINE_CONSTEXPR auto filter(F&& predicate) const& -> Option<T> {
    return copied().filter(std::forward<F>(predicate));
  }

  /**
   * Copys this option, if this is some and passes the predicate it
   * will return Some, else None
   */
  template<crab::ty::predicate<const T&> F>
  requires crab::ty::copy_constructible<T>
  CRAB_PURE_INLINE_CONSTEXPR auto filter(F&& predicate) & -> Option<T> {
    return copied().filter(std::forward<F>(predicate));
  }

  /**
   * Checks if this result contains a value, and if so does it also
   * match with the given predicate
   *
   * Option<i32>{10}.is_ok_and(crab::fn::even) -> true
   * Option<i32>{10}.is_ok_and(crab::fn::odd) -> false
   */
  template<crab::ty::predicate<const T&> F>
  CRAB_PURE_INLINE_CONSTEXPR auto is_some_and(F&& functor) const -> bool {
    return is_some() and std::invoke(functor, get_unchecked());
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
  CRAB_PURE_INLINE_CONSTEXPR auto zip(Option<Vals>... other) && -> Option<Tuple<T, Vals...>> {
    if (is_none()) {
      return crab::None{};
    }

    if ((... or other.is_none())) {
      return crab::None{};
    }

    return std::tuple<T, Vals...>{std::move(*this).unwrap(), std::move(other).unwrap()...};
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
  CRAB_PURE_INLINE_CONSTEXPR auto operator and(Option<S> other) && -> Option<S> {
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
  CRAB_PURE_INLINE_CONSTEXPR auto operator and(Option<S> other) const& -> Option<S>
    requires crab::ty::copy_constructible<T>
  {
    return copied() and std::move(other);
  }

  /**
   * Boolean or operation, if this option is some it will return that, if the
   * other option is some it will return that, else this returns none
   */
  CRAB_PURE_INLINE_CONSTEXPR auto operator or(Option other) && -> Option {
    if (is_none()) {
      return other;
    }

    return std::move(*this);
  }

  /**
   * Boolean or operation, if this option is some it will return that, if the
   * other option is some it will return that, else this returns none
   */
  CRAB_PURE_INLINE_CONSTEXPR auto operator or(Option other) const& -> Option requires crab::ty::copy_constructible<T>
  {
    return copied() or std::move(other);
  }

  /**
   * The same as or but if both options are some then this will be none
   */
  CRAB_PURE_INLINE_CONSTEXPR auto operator xor(Option other) && -> Option {
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
  CRAB_PURE_INLINE_CONSTEXPR auto operator xor(Option other) const& -> Option requires crab::ty::copy_constructible<T>
  {
    return copied() xor std::move(other);
  }

  ///
  /// Ordering Overloads
  ///

  template<typename S>
  CRAB_PURE_INLINE_CONSTEXPR auto operator==(const Option<S>& other) const -> bool {
    static_assert(std::equality_comparable_with<T, S>, "Cannot equate to options if the inner types are not equatable");
    if (is_none() or other.is_none()) {
      return is_none() == other.is_none();
    }

    return static_cast<bool>(get_unchecked() == other.get_unchecked());
  }

  template<typename S>
  CRAB_PURE_INLINE_CONSTEXPR auto operator!=(const Option<S>& other) const -> bool {
    static_assert(
      std::equality_comparable_with<T, S>,
      "Cannot equate to options if the inner types are not inverse equatable"
    );

    if (is_none() or other.is_none()) {
      return is_none() != other.is_none();
    }

    return static_cast<bool>(get_unchecked() != other.get_unchecked());
  }

  template<typename S>
  CRAB_PURE_INLINE_CONSTEXPR auto operator>(const Option<S>& other) const -> bool {
    static_assert(
      requires(const T& a, const S& b) {
        { a > b } -> crab::ty::convertible<bool>;
      },
      "Cannot compare to options if the inner types are not comparable with <"
    );

    if (is_none()) {
      return other.is_some();
    }

    if (other.is_none()) {
      return true;
    }

    return static_cast<bool>(get_unchecked() < other.get_unchecked());
  }

  template<typename S>
  CRAB_PURE_INLINE_CONSTEXPR auto operator>=(const Option<S>& other) const -> bool {
    static_assert(
      requires(const T& a, const S& b) {
        { a >= b } -> crab::ty::convertible<bool>;
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

    return static_cast<bool>(get_unchecked() >= other.get_unchecked());
  }

  template<typename S>
  CRAB_PURE_INLINE_CONSTEXPR auto operator<=(const Option<S>& other) const -> bool {
    static_assert(
      requires(const T& a, const S& b) {
        { a <= b } -> crab::ty::convertible<bool>;
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

    return static_cast<bool>(get_unchecked() <= other.get_unchecked());
  }

  template<typename S>
  CRAB_PURE_INLINE_CONSTEXPR auto operator<=>(const Option<S>& other) const -> std::partial_ordering {
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

    return static_cast<std::partial_ordering>(get_unchecked() <=> other.get_unchecked());
  }

  /**
   * Option<T> == None only if the former is none
   */
  CRAB_PURE_INLINE_CONSTEXPR auto operator==(const crab::None&) const -> bool {
    return is_none();
  }

  /**
   * Option<T> != None only if the former is some
   */
  CRAB_PURE_INLINE_CONSTEXPR auto operator!=(const crab::None&) const -> bool {
    return is_some();
  }

  /**
   * Option<T> > None only if the former is some
   */
  CRAB_PURE_INLINE_CONSTEXPR auto operator>(const crab::None&) const -> bool {
    return is_some();
  }

  /**
   * Option<T> >= None is always true
   */
  CRAB_PURE_INLINE_CONSTEXPR auto operator>=(const crab::None&) const -> bool {
    return true;
  }

  /**
   * Option<T> < None is never true
   */
  CRAB_PURE_INLINE_CONSTEXPR auto operator<(const crab::None&) const -> bool {
    return false;
  }

  /**
   * Option<T> <= None is never true
   */
  CRAB_PURE_INLINE_CONSTEXPR auto operator<=(const crab::None&) const -> bool {
    return is_none();
  }

private:

  Storage value;
};

template<typename T>
CRAB_INLINE_CONSTEXPR auto operator<<(std::ostream& os, const Option<T>& opt) -> std::ostream& {
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
  CRAB_PURE_INLINE_CONSTEXPR auto operator()(const Option<T>& opt) const -> crab::hash_code {
    if (opt.is_none()) {
      return 0;
    }

    return crab::hash_together<usize, T>(1, opt.get_unchecked());
  }
};

namespace crab {

  /**
   * Creates an Option<T> from some value T
   */
  template<typename T>
  CRAB_PURE_INLINE_CONSTEXPR auto some(std::type_identity_t<T>&& from) {
    return Option<T>{std::forward<T>(from)};
  }

  /**
   * Creates an Option<T> from some value T
   */
  CRAB_PURE_INLINE_CONSTEXPR auto some(auto from) {
    return Option<std::remove_cvref_t<decltype(from)>>{std::move(from)};
  }

  /**
   * Maps a boolean to an option if it is true
   */
  template<crab::ty::provider F>
  CRAB_PURE_INLINE_CONSTEXPR auto then(const bool cond, F&& func) {
    using Return = Option<crab::ty::functor_result<F>>;

    if (not cond) {
      return Return{};
    }

    return Return{std::invoke(func)};
  }

  /**
   * Maps a boolean to an option if it is false
   */
  template<crab::ty::provider F>
  CRAB_PURE_INLINE_CONSTEXPR auto unless(const bool cond, F&& func) {
    using Return = Option<crab::ty::functor_result<F>>;

    if (cond) {
      return Return{};
    }

    return Return{std::invoke(func)};
  }

  /**
   * Consumes given option and returns the contained value, will throw
   * if none found
   * @param from Option to consume
   */
  template<typename T>
  CRAB_PURE_INLINE_CONSTEXPR auto unwrap(Option<T>&& from) -> T {
    return std::forward<T>(from).unwrap();
  }

  namespace option {
    struct fallible final {
      template<typename... T>
      CRAB_PURE_INLINE_CONSTEXPR auto operator()(Tuple<T...> tuple) const {
        return Option<Tuple<T...>>{std::forward<Tuple<T...>>(tuple)};
      }

      template<typename PrevResults, crab::ty::provider F, typename... Rest>
      requires option_type<crab::ty::functor_result<F>>
      CRAB_PURE_INLINE_CONSTEXPR auto operator()(
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

      template<typename PrevResults, crab::ty::provider F, typename... Rest>
      requires(not option_type<crab::ty::functor_result<F>>)
      CRAB_PURE_INLINE_CONSTEXPR auto operator()(
        PrevResults tuple /* Tuple<T...>*/,
        F&& function,
        Rest&&... other_functions
      ) const {
        return operator()(
          std::tuple_cat(std::move(tuple), Tuple<crab::ty::functor_result<F>>(std::invoke(function))),
          std::forward<Rest>(other_functions)...
        );
      }

      template<typename PrevResults, typename V, typename... Rest>
      requires(not crab::ty::provider<V> and not option_type<V>)
      CRAB_PURE_INLINE_CONSTEXPR auto operator()(
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
      requires(not crab::ty::provider<V>)
      CRAB_PURE_INLINE_CONSTEXPR auto operator()(
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
  CRAB_PURE_INLINE_CONSTEXPR auto fallible(F&&... fallible) {
    return option::fallible{}(Tuple<>{}, std::forward<F>(fallible)...);
  }
} // namespace crab

// NOLINTEND(*explicit*)

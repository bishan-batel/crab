// Warning guards:

// ReSharper disable  CppNonExplicitConvertingConstructor
// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppNonExplicitConversionOperator
// NOLINTBEGIN(*explicit*)

#ifndef CRAB_OPT_OPTION_HPP
#define CRAB_OPT_OPTION_HPP

#include <concepts>
#include <cstddef>
#include <functional>
#include <iostream>
#include <type_traits>

#include "crab/preamble.hpp"
#include "crab/type_traits.hpp"
#include "crab/hash.hpp"
#include "./none.hpp"
#include "./impl/GenericStorage.hpp"
#include "./impl/RefStorage.hpp"
#include "crab/assertion/assert.hpp"

namespace crab::opt {

  /**
   * Storage selector for Option<T>, this type's public alias 'type' determines the structure
   * used as the internal storage for Option<T>
   */
  template<typename T>
  struct Storage final {
    using type =
      ty::conditional<ty::is_reference<T>, impl::RefStorage<ty::remove_reference<T>>, impl::GenericStorage<T>>;
  };

  template<typename T>
  using TStorage = typename Storage<T>::type;

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
      ty::different_than<T, None>,
      "Cannot make an option of crab::None, you will need to use some other unit "
      "type (eg. std::monostate, unit, or others)"
    );

    static_assert(
      ty::non_const<T>,
      "Using Option with a const parameter is discouraged as the const is transitive, consider using 'const "
      "Option<T>' rather than 'Option<const T>' "
    );

    /**
     * Contained inner type
     */
    using Contained = T;

    /**
     * Storage type for this data
     */
    using Storage = typename opt::Storage<T>::type;

    /**
     * Is the contained type T a reference type (immutable or mutable)
     */
    inline static constexpr bool is_ref = ty::is_reference<T>;

    /**
     * Is the contained type T a immutable reference type
     */
    inline static constexpr bool is_const_ref = is_ref and ty::is_const<T>;

    /**
     * Is the contained type T a mutable reference type
     */
    inline static constexpr bool is_mut_ref = is_ref and ty::non_const<T>;

    /**
     * Create an option that wraps Some(T)
     */
    CRAB_INLINE_CONSTEXPR Option(const T& from) noexcept(std::is_nothrow_copy_constructible_v<T>) requires(not is_ref)
        : storage{from} {}

    /**
     * Create an option that wraps Some(T)
     */
    CRAB_INLINE_CONSTEXPR Option(T&& from) noexcept(std::is_nothrow_move_constructible_v<T>):
        storage{mem::forward<T>(from)} {}

    /**
     * Create an empty option
     */
    CRAB_INLINE_CONSTEXPR Option(None = {}) noexcept: storage{none} {}

    /**
     * Conversion constructor for options of the form Option<T&> to be able to
     * convert implicitly to Option<RefMut<T>> (mainly for backwards
     * compatability)
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR operator Option<ref::RefMut<std::remove_cvref_t<T>>>() requires(is_mut_ref)
    {
      return map<ref::RefMut<std::remove_cvref_t<T>>>();
    }

    /**
     * Conversion constructor for options of the form Option<const T&>/Option<T&>
     * to be able to convert implicitly to Option<Ref<T>> (mainly for backwards
     * compatability)
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR operator Option<ref::Ref<std::remove_cvref_t<T>>>() requires(is_ref)
    {
      return map<ref::Ref<std::remove_cvref_t<T>>>();
    }

    /**
     * Conversion constructor for options of the form Option<Ref<T>> to be able to
     * convert implicitly to Option<const T&>
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR operator Option<crab::ty::crab_ref_decay<T>>() requires(crab::ty::crab_ref<T>)
    {
      return map<const T&>();
    }

    /**
     * Conversion constructor for options of the form Option<Ref<T>> to be able to
     * convert implicitly to Option<T&>
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR operator Option<crab::ty::crab_ref_decay<T>>() requires(crab::ty::crab_ref_mut<T>)
    {
      return map<T&>();
    }

    /**
     * Reassign option to Some(T),
     * If this option previously contained Some(K), the previous value is
     * discarded and is replaced by Some(T)
     */
    CRAB_INLINE_CONSTEXPR auto operator=(T&& from) -> Option& requires(not is_ref) {
      storage = mem::forward<T>(from);
      return *this;
    }

    /**
     * Reassign option to None,
     * If this option previously contained Some(K), the previous value is
     * discarded and is replaced by Some(T)
     */
    CRAB_INLINE_CONSTEXPR auto operator=(None) -> Option& {
      storage = none;
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
    CRAB_NODISCARD_INLINE_CONSTEXPR explicit operator bool() const {
      return is_some();
    }

    /**
     * Whether this option contains a value
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto is_some() const -> bool {
      return storage.in_use();
    }

    /**
     * Whether this option does not contain a value
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto is_none() const -> bool {
      return not storage.in_use();
    }

    /**
     * Converts a 'const Option<T>&' into a Option<const T&>, to give
     * optional access to the actual referenced value inside.
     *
     * This function returns Option<const T&>
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto as_ref() const& -> Option<const T&> requires(not is_ref)
    {
      if (is_none()) {
        return none;
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto as_mut() & -> Option<T&> requires(not is_ref)
    {
      if (is_none()) {
        return none;
      }

      return Option<T&>{get_unchecked()};
    }

    /**
     * Consumes inner value and returns it, if this option was none this
     * will instead return a new default constructed T
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto take_or_default() && -> T requires ty::default_constructible<T>
    {
      if (is_none()) {
        return T{};
      }

      return mem::move(*this).unwrap();
    }

    /**
     * Takes the contained value (like Option<T>::unwrap()) if
     * exists, else returns a default value
     * @param default_value
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto take_or(T default_value) && -> T {
      if (is_some()) {
        return mem::move(*this).unwrap();
      }

      return default_value;
    }

    /**
     * Takes the contained value (like Option<T>::unwrap()) if
     * exists, else uses 'F' to compute & create a default value
     * @param default_generator Function to create the default value
     */
    template<ty::provider<T> F>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto take_or(F&& default_generator) && -> T {
      if (is_some()) {
        return mem::move(*this).unwrap();
      }

      return std::invoke(default_generator);
    }

    /**
     * Is this option has a value, return a copy of that value. if
     * this opton was none then this returns a default constructed T
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto get_or_default() const -> T requires ty::copy_constructible<T>
    {
      return copied().take_or_default();
    }

    /**
     * Gets the contained value if exists, else returns a default value
     * @param default_value
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto get_or(T default_value) const -> T requires ty::copy_constructible<T>
    {
      return copied().take_or(mem::forward<T>(default_value));
    }

    /**
     * Gets the contained value if exists, else computes a default value
     * with 'F' and returns
     * @param default_generator Function to create the default value
     */
    template<ty::provider<T> F>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto get_or(F&& default_generator) const -> T requires ty::copy_constructible<T>
    {
      static_assert(std::is_invocable_r_v<T, F>, "A function with Option<T>::get_or must return T");
      return copied().take_or(mem::forward<F>(default_generator));
    }

    /**
     * Takes value out of the option and returns it, will error if option
     * is none, After this, the value has been 'taken out' of this option, after
     * this method is called this option is 'None'
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto unwrap(SourceLocation loc = SourceLocation::current()) && -> T {
      debug_assert_transparent(is_some(), loc, "Cannot unwrap a none option");

      return mem::move(storage).value();
    }

    /**
     * Returns a mutable reference to the contained Some value inside, if
     * this option is none this will panic & crash.
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto get_unchecked(SourceLocation loc = SourceLocation::current()) -> T& {
      debug_assert_transparent(is_some(), loc, "Cannot get_unchecked a none option");

      return storage.value();
    }

    /**
     * Returns a const reference to the contained Some value inside, if
     * this option is none this will panic & crash.
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto get_unchecked(SourceLocation loc = SourceLocation::current()) const
      -> const T& {
      debug_assert_transparent(is_some(), loc, "Cannot get_unchecked a none option");

      return storage.value();
    }

    /**
     * Creates a Result<T, E> from this given option, where "None" is
     * expanded to some error given.
     * @tparam E Error Type
     * @param error Error to replace an instance of None
     */
    template<typename E>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto ok_or(E&& error) const
      -> result::Result<T, E> requires ty::copy_constructible<T>
    {
      if (is_some()) {
        return result::Ok<T>{get_unchecked()};
      }

      return result::Err<E>{mem::forward<E>(error)};
    }

    /**
     * Creates a Result<T, E> from this given option, where "None" is
     * expanded to some error given.
     * @tparam E Error Type
     * @param error_generator Function to generate an error to replace "None".
     */
    template<typename E, ty::provider<E> F>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto ok_or(F&& error_generator) const
      -> result::Result<T, E> requires ty::copy_constructible<T>
    {
      if (is_some()) {
        return result::Ok<T>{get_unchecked()};
      }

      return result::Err<E>{std::invoke(error_generator)};
    }

    /**
     * Converts this Result<T, E> from this given option, where "None" is
     * expanded to some error given. This will invalidate the Option<T> after
     * call, just like every other opt::take_* function.
     * @tparam E Error Type
     * @param error Error to replace an instance of None
     */
    template<typename E>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto take_ok_or(E error) && -> result::Result<T, E> {
      if (is_some()) {
        return result::Result<T, E>{result::Ok<T>{mem::move(*this).unwrap()}};
      }

      return result::Result<T, E>(result::Err<E>{mem::forward<E>(error)});
    }

    /**
     * Creates a Result<T, E> from this given option, where "None" is
     * expanded to some error given.
     * @tparam E Error Type
     * @param error_generator Function to generate an error to replace "None".
     */
    template<typename E, ty::provider<E> F>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto take_ok_or(F&& error_generator) && -> result::Result<T, E> {
      if (is_some()) {
        return result::Result<T, E>{
          result::Ok<T>{mem::move(*this).unwrap()},
        };
      }

      return result::Result<T, E>{
        result::Err<E>{std::invoke(error_generator)},
      };
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto map(F&& mapper) && {
      using Returned = Option<ty::functor_result<F, T>>;

      return is_some() ? Returned{std::invoke(mapper, mem::move(*this).unwrap())} : Returned{};
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
    template<ty::mapper<T> F>
    requires ty::copy_constructible<T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto map(F&& mapper) const& {
      return copied().map(mem::forward<F>(mapper));
    }

    template<typename Into>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto map() && -> Option<Into> {
      static_assert(ty::convertible<T, Into>, "'Option<T>::map<Into>()' can only be done if T is convertible to Into");

      return mem::move(*this).map([](T&& value) -> Into { return static_cast<Into>(mem::forward<T>(value)); });
    }

    template<typename Into>
    requires ty::copy_constructible<T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto map() const& -> Option<Into> {
      return copied().template map<Into>();
    }

    /**
     * Shorthand for calling .map(...).flatten()
     */
    template<ty::mapper<T> F>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto flat_map(F&& mapper) && {
      using Returned = ty::functor_result<F, T>;
      static_assert(ty::option_type<Returned>, "The function passed to flat_map must return an Option");

      if (is_some()) {
        return Returned{std::invoke(mapper, mem::move(*this).unwrap())};
      }

      return Returned{None{}};
    }

    /**
     * Shorthand for calling .map(...).flatten()
     */
    template<ty::mapper<T> F>
    requires ty::copy_constructible<T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto flat_map(F&& mapper) const& {
      return copied().flat_map(mem::forward<F>(mapper));
    }

    /**
     * Equivalent of flat_map but for the 'None' type
     */
    template<ty::provider F>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto or_else(F&& mapper) && {
      using Returned = ty::functor_result<F>;

      static_assert(ty::option_type<Returned>, "The function passed to or_else must return an Option");

      if (is_some()) {
        return Returned{mem::move(*this).unwrap()};
      }

      return Returned{std::invoke(mapper)};
    }

    /**
     * Equivalent of flat_map but for the 'None' type
     */
    template<ty::provider F>
    requires ty::copy_constructible<T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto or_else(F&& mapper) const& {
      return copied().or_else(mem::forward<F>(mapper));
    }

    /**
     * If this option is of some type Option<Option<T>>, this will flatten
     * it to a single Option<T>
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto flatten() && -> T requires ty::option_type<T>
    {
      if (is_none()) {
        return None{};
      }

      return mem::move(*this).unwrap();
    }

    /**
     * If this option is of some type Option<Option<T>>, this will flatten
     * it to a single Option<T>
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto flatten() const& -> T requires ty::option_type<T> and ty::copy_constructible<T>

    {
      return copied().flatten();
    }

    /**
     * Copies this option and returns, use this before map if you do not
     * want to consume the option.
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto copied() const -> Option requires ty::copy_constructible<T>
    {
      static_assert(ty::copy_constructible<T>, "Cannot call copied() on an option with a non copy-cosntructible type");

      if (is_some()) {
        return Option<T>{get_unchecked()};
      }

      return None{};
    }

    /**
     * Consumes this option, if this is some and passes the predicate it
     * will return Some, else None
     */
    template<ty::predicate<const T&> F>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto filter(F&& predicate) && -> Option {
      if (is_none()) {
        return None{};
      }

      T value{mem::move(*this).unwrap()};

      const bool passed{
        static_cast<bool>(std::invoke(predicate, static_cast<const T&>(value))),
      };

      if (not passed) {
        return None{};
      }

      return Option{mem::forward<Contained>(value)};
    }

    /**
     * Copys this option, if this is some and passes the predicate it
     * will return Some, else None
     */
    template<crab::ty::predicate<const T&> F>
    requires crab::ty::copy_constructible<T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto filter(F&& predicate) const& -> Option<T> {
      return copied().filter(mem::forward<F>(predicate));
    }

    /**
     * Copys this option, if this is some and passes the predicate it
     * will return Some, else None
     */
    template<crab::ty::predicate<const T&> F>
    requires crab::ty::copy_constructible<T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto filter(F&& predicate) & -> Option<T> {
      return copied().filter(mem::forward<F>(predicate));
    }

    /**
     * Checks if this result contains a value, and if so does it also
     * match with the given predicate
     *
     * Option<i32>{10}.is_ok_and(crab::fn::even) -> true
     * Option<i32>{10}.is_ok_and(crab::fn::odd) -> false
     */
    template<crab::ty::predicate<const T&> F>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto is_some_and(F&& functor) const -> bool {
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto zip(Option<Vals>... other) && -> Option<Tuple<T, Vals...>> {
      if (is_none()) {
        return None{};
      }

      if ((... or other.is_none())) {
        return None{};
      }

      return Tuple<T, Vals...>{mem::move(*this).unwrap(), mem::move(other).unwrap()...};
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator and(Option<S> other) && -> Option<S> {
      if (is_none()) {
        return None{};
      }

      return other;
    }

    /**
     * Boolean 'and' operation on two options, this will return none if either of
     * the two options are none, and if both are some this will return the
     * contained value of the second
     */
    template<typename S>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator and(Option<S> other) const& -> Option<S>
      requires ty::copy_constructible<T>
    {
      return copied() and mem::move(other);
    }

    /**
     * Boolean or operation, if this option is some it will return that, if the
     * other option is some it will return that, else this returns none
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator or(Option other) && -> Option {
      if (is_none()) {
        return other;
      }

      return mem::move(*this);
    }

    /**
     * Boolean or operation, if this option is some it will return that, if the
     * other option is some it will return that, else this returns none
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator or(Option other) const& -> Option requires ty::copy_constructible<T>
    {
      return copied() or mem::move(other);
    }

    /**
     * The same as or but if both options are some then this will be none
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator xor(Option other) && -> Option {
      if (is_none()) {
        return other;
      }

      if (other.is_some()) {
        return None{};
      }

      return mem::move(*this);
    }

    /**
     * The same as or but if both options are some then this will be none
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator xor(Option other) const& -> Option
      requires crab::ty::copy_constructible<T>
    {
      return copied() xor mem::move(other);
    }

    ///
    /// Ordering Overloads
    ///

    template<typename S>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator==(const Option<S>& other) const -> bool {
      static_assert(
        std::equality_comparable_with<T, S>,
        "Cannot equate to options if the inner types are not equatable"
      );
      if (is_none() or other.is_none()) {
        return is_none() == other.is_none();
      }

      return static_cast<bool>(get_unchecked() == other.get_unchecked());
    }

    template<typename S>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator!=(const Option<S>& other) const -> bool {
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator>(const Option<S>& other) const -> bool {
      static_assert(
        requires(const T& a, const S& b) {
          { a > b } -> ty::convertible<bool>;
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator>=(const Option<S>& other) const -> bool {
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator<=(const Option<S>& other) const -> bool {
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator<=>(const Option<S>& other) const -> std::partial_ordering {
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
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator==(const None&) const -> bool {
      return is_none();
    }

    /**
     * Option<T> != None only if the former is some
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator!=(const None&) const -> bool {
      return is_some();
    }

    /**
     * Option<T> > None only if the former is some
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator>(const None&) const -> bool {
      return is_some();
    }

    /**
     * Option<T> >= None is always true
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator>=(const None&) const -> bool {
      return true;
    }

    /**
     * Option<T> < None is never true
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator<(const None&) const -> bool {
      return false;
    }

    /**
     * Option<T> <= None is never true
     */
    CRAB_NODISCARD_INLINE_CONSTEXPR auto operator<=(const None&) const -> bool {
      return is_none();
    }

  private:

    Storage storage;
  };
}

// TODO: change this into usage for std::formatter or fmt::formatter
template<typename T>
CRAB_INLINE_CONSTEXPR auto operator<<(std::ostream& os, const ::crab::opt::Option<T>& opt) -> std::ostream& {
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
struct std::hash<::crab::opt::Option<T>> /*NOLINT*/ {
  CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(const ::crab::opt::Option<T>& opt) const -> crab::hash_code {
    if (opt.is_none()) {
      return crab::hash(false);
    }

    return crab::hash_together(true, opt.get_unchecked());
  }
};

namespace crab::prelude {
  using opt::Option;
}

CRAB_PRELUDE_GUARD;

#endif

#ifndef CRAB_ASSERTION_PANIC_HPP
#include "crab/assertion/panic.hpp"
#endif

// NOLINTEND(*explicit*)

/// @file crab/opt/Option.hpp
/// @ingroup opt

// ReSharper disable  CppNonExplicitConvertingConstructor
// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppNonExplicitConversionOperator

#pragma once

#include <concepts>

#include "crab/assertion/check.hpp"
#include "crab/collections/Tuple.hpp"
#include "crab/core/unsafe.hpp"
#include "crab/opt/none.hpp"

#include "crab/opt/impl/GenericStorage.hpp"
#include "crab/opt/impl/RefStorage.hpp"

#include "crab/hash/hash.hpp"
#include "crab/opt/concepts.hpp"
#include "crab/ref/ref.hpp"
#include "crab/ref/implicit_cast.hpp"
#include "crab/result/forward.hpp"
#include "crab/str/str.hpp"
#include "crab/ty/crab_ref_decay.hpp"
#include "crab/ty/functor.hpp"
#include "fmt/base.h"

namespace crab::opt {
  /// Storage selector for Option<T>, this type's public alias 'type' determines the structure
  /// used as the internal storage for Option<T>.
  ///
  /// TODO: Document strict requirements for what a type should be able to do
  ///
  /// @ingroup opt
  template<typename T>
  struct Storage final {
    /// Alias for the type used as storage for Option<T>
    /// @hideinitializer
    using type = impl::GenericStorage<T>;
  };

  /// Niche optimisation for Option's of reference typesj
  /// @copydoc Storage
  /// @ingroup opt
  template<typename T>
  struct Storage<T&> final {
    /// Reference specialization storage type
    /// @hideinitializer
    using type = impl::RefStorage<T&>;
  };

  /// Tagged union type between T and unit, alternative to std::optional<T>. For more details, read topic
  /// [Option](#opt).
  ///
  /// Note that unlike most types in crab, a moved-from option is valid to be used - as it is guarenteed that an option
  /// that is moved results in an empty option.
  ///
  /// @ingroup opt
  /// @ingroup prelude
  template<typename T>
  class Option final {
  public:

    /// Cannot have an optional None due to overload resolution issues
    static_assert(
      ty::different_than<T, None>,
      "Cannot make an option of crab::None, you will need to use some other unit "
      "type (eg. std::monostate, unit, or others)"
    );

    /// Option<const T> is not allowed, use instead const Option<T>
    static_assert(
      ty::non_const<T>,
      "Using Option with a const parameter is discouraged as the const is transitive, consider using 'const "
      "Option<T>' rather than 'Option<const T>' "
    );

    /// Inner type of this option, exposed for easier templated access.
    using Contained = T;

    /// Storage type for this data
    using Storage = typename opt::Storage<T>::type;

    /// shallow assertion about validity of the type we use as storage
    static_assert(
      is_storage_type<Storage, T>,
      "Provided storage type for this option is ill-formed for this type T, note this is most likely due to incorrect "
      "implementation of custom storage types for Option."
    );

    /// Is the contained type T a reference type (immutable or mutable)
    inline static constexpr bool is_ref = ty::is_reference<T>;

    /// Is the contained type T a immutable reference type
    inline static constexpr bool is_const_ref = is_ref and ty::is_const<T>;

    /// Is the contained type T a mutable reference type
    inline static constexpr bool is_mut_ref = is_ref and ty::non_const<T>;

    /// Whether this option allows implicit copies. If this is false you are required to either explicitly called
    /// Option::copied before using some method (like 'map' or 'filter'), or are required to move the value before
    /// calling said method. Implicit copies are only allowed if the contained type is trivially copyable, therefore
    /// explicit moves would not be worth the visual complexity.
    ///
    /// implicit_copy_allowed is transitive through nested options.
    ///
    /// Option<Option<Option<...<T>...>>> for any number of wrapping Option's all have implicit copies allowed if the
    /// inner type T is implicit copyable
    inline static constexpr bool implicit_copy_allowed = []() {
      if (is_ref or std::is_trivially_copyable_v<T>) {
        return true;
      }

      if constexpr (option_type<T>) {
        return T::implicit_copy_allowed;
      }

      return false;
    }();

    // NOLINTBEGIN(*explicit*)

    /// Create an option that wraps Some(T)
    CRAB_INLINE constexpr Option(const T& from) noexcept(std::is_nothrow_copy_constructible_v<T>) requires(not is_ref)
        : storage{from} {}

    /// Create an option that wraps Some(T)
    CRAB_INLINE constexpr Option(T&& from) noexcept(std::is_nothrow_move_constructible_v<T>):
        storage{mem::forward<T>(from)} {}

    /// Create an empty Option
    CRAB_INLINE constexpr Option(None = {}) noexcept: storage{none} {}

    /// Conversion constructor for options of the form Option<T&> to be able to
    /// convert implicitly to Option<RefMut<T>> (mainly for backwards compatability)
    [[nodiscard]] CRAB_INLINE constexpr operator Option<ref::RefMut<std::remove_cvref_t<T>>>() requires(is_mut_ref)
    {
      return map<ref::RefMut<std::remove_cvref_t<T>>>();
    }

    /// Conversion constructor for options of the form Option<const T&>/Option<T&>
    /// to be able to convert implicitly to Option<Ref<T>> (mainly for backwards
    /// compatability)
    [[nodiscard]] CRAB_INLINE constexpr operator Option<ref::Ref<std::remove_cvref_t<T>>>() requires(is_ref)
    {
      return map<ref::Ref<std::remove_cvref_t<T>>>();
    }

    /// Conversion constructor for options of the form Option<Ref<T>> to be able to
    /// convert implicitly to Option<const T&>
    [[nodiscard]] CRAB_INLINE constexpr operator Option<ty::crab_ref_decay<T>>() requires(ty::crab_ref<T>)
    {
      return map<const T&>();
    }

    /// Conversion constructor for options of the form Option<Ref<T>> to be able to
    /// convert implicitly to Option<T&>
    [[nodiscard]] CRAB_INLINE constexpr operator Option<ty::crab_ref_decay<T>>() requires(ty::crab_ref_mut<T>)
    {
      return map<T&>();
    }

    // NOLINTEND(*explicit*)

    /// Reassign option to Some(T),
    /// If this option previously contained Some(K), the previous value is
    /// discarded and is replaced by Some(T)
    CRAB_INLINE constexpr auto operator=(T&& from) -> Option& requires(not is_ref)
    {
      storage = mem::forward<T>(from);
      return *this;
    }

    /// Reassign option to None,
    /// If this option previously contained Some(K), the previous value is
    /// discarded and is replaced by Some(T).
    CRAB_INLINE constexpr auto operator=(None) -> Option& {
      storage = none;
      return *this;
    }

    /// Whether this option has a contained value or not, alias to [is_some](#Option::is_some)
    [[nodiscard]] CRAB_INLINE constexpr explicit operator bool() const {
      return is_some();
    }

    /// Whether this option contains a value
    [[nodiscard]] CRAB_INLINE constexpr auto is_some() const -> bool {
      return storage.in_use();
    }

    /// Whether this option does not contain a value
    [[nodiscard]] CRAB_INLINE constexpr auto is_none() const -> bool {
      return not storage.in_use();
    }

    /// Converts a 'const Option<T>&' into a Option<const T&>, to give
    /// optional access to the actual referenced value inside.
    /// inside. You cannnot call this method on anything but a lvalue.
    ///
    /// @returns Option<const T&>
    [[nodiscard]] CRAB_INLINE constexpr auto as_ref() const& -> Option<const T&> requires(not is_ref)
    {
      if (is_none()) {
        return none;
      }

      return Option<const T&>{get_unchecked(unsafe)};
    }

    /// Converts a 'Option<T>&' into a
    /// Option<T&>, to give optional access to the actual referenced value
    /// inside. You cannnot call this method on anything mut a mutable lvalue.
    ///
    /// @returns Option<T&>
    [[nodiscard]] CRAB_INLINE constexpr auto as_mut() & -> Option<T&> requires(not is_ref)
    {
      if (is_none()) {
        return none;
      }

      return Option<T&>{get_unchecked(unsafe)};
    }

    /// Copies this option and returns. This function is the same as calling the copy constructor, but can help to clean
    /// up code as its a explicit method call. Many methods in Option require that the option be an rvalue (a moved from
    /// state) to perform in order to prevent costly implicit copies. In these cases, if you do not wish to move the
    /// underlying option you would simply call this method. However, if the type this option contains is *trivially
    /// copyable*, then methods that would normally require the option to be in a moved-from state will instead perform
    /// an implicit copy.
    ///
    /// # Examples
    ///
    /// ```cpp
    /// Option<String> msg = "some message";
    ///
    /// Option<String> msg_copy = ms.copied();
    ///
    /// Option<String> substring{
    ///   msg
    ///     .copied()
    ///     .map([](String s) { return s.substr(3); })
    /// };
    ///
    /// ```
    [[nodiscard]] CRAB_INLINE constexpr auto copied() const -> Option requires ty::copy_constructible<T>
    {
      static_assert(ty::copy_constructible<T>, "Cannot call copied() on an option with a non copy-cosntructible type");

      if (is_some()) {
        return Option<T>{get_unchecked(unsafe)};
      }

      return {};
    }

    /// Consumes inner value and returns it, if this option was none this
    /// will instead return a new default constructed T
    [[nodiscard]] CRAB_INLINE constexpr auto take_or_default() && -> T requires ty::default_constructible<T>
    {
      if (is_none()) {
        return T();
      }

      return mem::move(*this).unwrap();
    }

    /// Takes the contained value (like Option<T>::unwrap()) if
    /// exists, else returns a default value
    /// @param default_value
    [[nodiscard]] CRAB_INLINE constexpr auto take_or(T default_value) && -> T {
      if (is_some()) {
        return mem::move(*this).unwrap();
      }

      return default_value;
    }

    /// Takes the contained value (like Option<T>::unwrap()) if
    /// exists, else uses 'F' to compute & create a default value
    /// @param default_generator Function to create the default value
    template<ty::provider<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto take_or(F&& default_generator) && -> T {
      if (is_some()) {
        return mem::move(*this).unwrap();
      }

      return std::invoke(default_generator);
    }

    /// Rvalue qualified form of Option::get_or_default, calling this on an option that has already been moved to be a
    /// rvalue will make this act as an alias to Option::take_or_default
    [[nodiscard]] CRAB_INLINE constexpr auto get_or_default() && -> T {
      return mem::move(*this).take_or_default();
    }

    /// Is this option has a value, return a copy of that value. if
    /// this opton was none then this returns a default constructed T.
    ///
    /// This version will perform an implicit copy if this contains a value,
    /// if possible try to use Option::take_or_default instead.
    [[nodiscard]] CRAB_INLINE constexpr auto get_or_default() const& -> T requires implicit_copy_allowed
    {
      return copied().take_or_default();
    }

    /// Gets the contained value if exists, else returns a default value. Note for rvaleus this is
    /// equivalent to Object::take_or
    [[nodiscard]] CRAB_INLINE constexpr auto get_or(T default_value) && -> T {
      return mem::move(*this).take_or(mem::forward<T>(default_value));
    }

    /// Gets the contained value if exists, else returns a default value
    /// @param default_value
    [[nodiscard]] CRAB_INLINE constexpr auto get_or(T default_value) const& -> T requires implicit_copy_allowed
    {
      return copied().take_or(mem::forward<T>(default_value));
    }

    /// Returns a copy of the contained value if it exists, if this option contains 'none' then it will return the value
    /// returned by invoking the given functor. Note that this method used on an rvalue is equivalent to
    /// calling Option::take_or.
    ///
    /// @param default_generator Function that returns the 'default' value if this option does not contain anything
    template<ty::provider<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto get_or(F&& default_generator) && -> T {
      return mem::move(*this).take_or(mem::forward<F>(default_generator));
    }

    /// Gets the contained value if exists, else computes a default value
    /// with 'F' and returns. This will implicitly copy the inner value if it exists, therefore it is only valid to call
    /// on option types allowing implicit copies. In all other cases use instead Option::take_or with crab::move or
    /// Option::copied.
    ///
    /// @param default_generator Function to create the default value
    template<ty::provider<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto get_or(F&& default_generator) const& -> T requires implicit_copy_allowed
    {
      return copied().take_or(mem::forward<F>(default_generator));
    }

    /// @{

    /// Takes value out of the option and returns it, will error if option
    /// is none, After this, the value has been 'taken out' of this option, after
    /// this method is called this option is 'None'.
    ///
    /// # Panics
    /// This will panic if called on an option containing None. If you wish to elide this check (in release mode), use
    /// Option::unwrap_unchecked
    [[nodiscard]] CRAB_INLINE constexpr auto unwrap(const SourceLocation& loc = SourceLocation::current()) && -> T {
      crab_check_with_location(is_some(), loc, "Cannot unwrap a none option");

      return mem::move(storage).value();
    }

    /// Implicit copy form of unwrap.
    [[nodiscard]] CRAB_INLINE constexpr auto unwrap(const SourceLocation& loc = SourceLocation::current()) const& -> T
      requires implicit_copy_allowed
    {
      return copied().unwrap(loc);
    }

    /// }@

    /// @{

    /// Moves out the inner value contained in this option, this will panic if this option is none on debug and will
    /// result in undefined behavior in release. It is advised to use Object::unwrap instead unless you need to squeeze
    /// performance.
    ///
    /// # Panics
    /// If the object is 'none' / empty this will panic on debug.
    [[nodiscard]] CRAB_INLINE constexpr auto unwrap_unchecked(
      unsafe_fn,
      const SourceLocation& loc = SourceLocation::current()
    ) && -> T {
      crab_dbg_check_with_location(is_some(), loc, "Cannot unwrap_unchecked a none option");

      return mem::move(storage).value();
    }

    /// Trivial / implicit copy form of unwrap_unchecked.
    [[nodiscard]] CRAB_INLINE constexpr auto unwrap_unchecked(
      unsafe_fn,
      const SourceLocation& loc = SourceLocation::current()
    ) const& -> T requires implicit_copy_allowed
    {
      return copied().unwrap_unchecked(unsafe, loc);
    }

    /// }@

    /// Returns a reference to the contained Some value inside. This only works if not operating on an rvalue, in which
    /// case you should use Option::unwrap
    ///
    /// # Panics
    ///
    /// This function will panic if there is no value contained within this option.
    ///
    /// If you would like one that only panics in debug mode, see Option::get_unchecked
    [[nodiscard]] CRAB_INLINE constexpr auto get(const SourceLocation& loc = SourceLocation::current()) & -> T& {
      crab_check_with_location(is_some(), loc, "Cannot 'get' a none option");

      return storage.value();
    }

    /// @copydoc Option::get
    [[nodiscard]] CRAB_INLINE constexpr auto get(
      const SourceLocation& loc = SourceLocation::current()
    ) const& -> const T& {
      crab_check_with_location(is_some(), loc, "Cannot 'get' a none option");

      return storage.value();
    }

    /// Unsafe variant of Option::get, however this variant is meant for more performance critical areas and will elide
    /// the panic-check during a release build.
    ///
    /// # Panics
    /// This will panic if called on an option containing nothing *on debug mode only*.
    [[nodiscard]] CRAB_INLINE constexpr auto get_unchecked(
      unsafe_fn,
      const SourceLocation& loc = SourceLocation::current()
    ) & -> T& {
      crab_dbg_check_with_location(is_some(), loc, "Cannot get_unchecked a none option");

      return storage.value();
    }

    /// @copydoc Option::get_unchecked
    [[nodiscard]] CRAB_INLINE constexpr auto get_unchecked(
      unsafe_fn,
      const SourceLocation& loc = SourceLocation::current()
    ) const& -> const T& {
      crab_dbg_check_with_location(is_some(), loc, "Cannot get_unchecked a none option");

      return storage.value();
    }

    /// Creates a Result<T, E> from this given option, where "None" is
    /// expanded to some error given. This performs an implicit copy - if the given Option<T> does not allow implicit
    /// copies then use Option::take_or.
    /// @tparam E Error Type
    /// @param error Error to replace an instance of None
    template<typename E>
    [[nodiscard]] CRAB_INLINE constexpr auto ok_or(E error) const& -> result::Result<T, E>
      requires implicit_copy_allowed
    {
      return copied().take_ok_or(mem::forward<E>(error));
    }

    /// Rvalue qualified 'ok_or', note that this is simply an alias for Option::take_ok_or.
    template<typename E>
    [[nodiscard]] CRAB_INLINE constexpr auto ok_or(E error) && -> result::Result<T, E> requires implicit_copy_allowed
    {
      return mem::move(*this).take_ok_or(mem::forward<E>(error));
    }

    /// Creates a Result<T, E> from this given option, where "None" is
    /// expanded to some error given. This will perform an implicit copy, if that is not allowed
    /// then use Option::take_ok_or
    /// @tparam E Error Type
    /// @param error_generator Function to generate an error to replace "None".
    template<typename E, ty::provider<E> F>
    [[nodiscard]] CRAB_INLINE constexpr auto ok_or(F&& error_generator) const& -> result::Result<T, E>
      requires implicit_copy_allowed
    {
      return copied().take_ok_or(mem::forward<F>(error_generator));
    }

    /// Rvalue qualified form of Option::ok_or, note that when calling this on an rvalue (moved value), this is simply
    /// an alias for Option::take_ok_or
    template<typename E, ty::provider<E> F>
    [[nodiscard]] CRAB_INLINE constexpr auto ok_or(F&& error_generator) && -> result::Result<T, E>
      requires implicit_copy_allowed
    {
      return mem::move(*this).take_ok_or(mem::forward<F>(error_generator));
    }

    /// Converts this Result<T, E> from this given option, where "None" is
    /// expanded to some error given. This will invalidate the Option<T> after
    /// call, just like every other opt::take_* function.
    /// @tparam E Error Type
    /// @param error Error to replace an instance of None
    template<typename E>
    [[nodiscard]] CRAB_INLINE constexpr auto take_ok_or(E error) && -> result::Result<T, E> {
      if (is_some()) {
        return result::Result<T, E>{result::Ok<T>{mem::move(*this).unwrap_unchecked(unsafe)}};
      }

      return result::Result<T, E>(result::Err<E>{mem::forward<E>(error)});
    }

    /// Creates a Result<T, E> from this given option, where "None" is
    /// expanded to some error given.
    /// @tparam E Error Type
    /// @param error_generator Function to generate an error to replace "None".
    template<typename E, ty::provider<E> F>
    [[nodiscard]] CRAB_INLINE constexpr auto take_ok_or(F&& error_generator) && -> result::Result<T, E> {
      if (is_some()) {
        return result::Result<T, E>{
          result::Ok<T>{mem::move(*this).unwrap_unchecked(unsafe)},
        };
      }

      return result::Result<T, E>{
        result::Err<E>{std::invoke(error_generator)},
      };
    }

    /// @name Monadic Operations
    /// @{

    /// Transform function to an Option<T> -> Option<K>, if this option is
    /// None it will simply return None, but if it is Some(T), this will take the
    /// value out of this option and call the given 'mapper' function with it, the
    /// returned value is then wrapped and this function returns Some
    ///
    /// # Examples
    ///
    /// ```cpp
    /// crab_check(Option<f32>{crab::none}
    ///   .map([](f32 x) { return static_cast<i32>(x); }) // returns Option<i32>
    ///   .is_none()
    /// );
    /// crab_check(Option<i32>{420}
    ///   .map([](i32 x) { return std::tostring(x); }) // returns a Option<String>
    ///   .take_unchecked() == "420"
    /// );
    /// ```
    template<ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto map(F&& mapper) && {
      using Returned = Option<ty::functor_result<F, T>>;

      return is_some() ? Returned{std::invoke(mapper, mem::move(*this).unwrap_unchecked(unsafe))} : Returned{};
    }

    /// Transform function to an Option<T> -> Option<K>, if this option is
    /// None it will simply return None, but if it is Some(T), this will take the
    /// value out of this option and call the given 'mapper' function with it, the
    /// returned value is then wrapped and this function returns Some
    ///
    /// # Examples
    ///
    /// ```cpp
    /// crab_check(Option<f32>{crab::none}
    ///   .map([](f32 x) { return static_cast<i32>(x); }) // returns Option<i32>
    ///   .is_none()
    /// );
    /// crab_check(Option<i32>{420}
    ///   .map([](i32 x) { return std::tostring(x); }) // returns a Option<String>
    ///   .take_unchecked() == "420"
    /// );
    /// ```
    template<typename Into>
    [[nodiscard]] CRAB_INLINE constexpr auto map() && -> Option<Into> {
      static_assert(ty::convertible<T, Into>, "'Option<T>::map<Into>()' can only be done if T is convertible to Into");

      return mem::move(*this).map([](T&& value) -> Into { return static_cast<Into>(mem::forward<T>(value)); });
    }

    /// TODO: docs
    template<ty::mapper<T> F>
    requires implicit_copy_allowed
    [[nodiscard]] CRAB_INLINE constexpr auto map(F&& mapper) const& {
      return copied().map(mem::forward<F>(mapper));
    }

    /// TODO: docs
    template<typename Into>
    requires implicit_copy_allowed
    [[nodiscard]] CRAB_INLINE constexpr auto map() const& -> Option<Into> {
      return copied().template map<Into>();
    }

    /// Shorthand for calling .map(...).flatten()
    template<ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto flat_map(F&& mapper) && {
      using Returned = ty::functor_result<F, T>;
      static_assert(option_type<Returned>, "The function passed to flat_map must return an Option");

      if (is_some()) {
        return Returned{std::invoke(mem::forward<F>(mapper), mem::move(*this).unwrap_unchecked(unsafe))};
      }

      return Returned{None{}};
    }

    /// TODO: doc
    template<ty::mapper<T> F>
    requires implicit_copy_allowed
    [[nodiscard]] CRAB_INLINE constexpr auto flat_map(F&& mapper) const& {
      return copied().flat_map(mem::forward<F>(mapper));
    }

    /// Equivalent of flat_map but for the 'None' type
    template<ty::provider F>
    [[nodiscard]] CRAB_INLINE constexpr auto or_else(F&& mapper) && {
      using Returned = ty::functor_result<F>;

      static_assert(option_type<Returned>, "The function passed to or_else must return an Option");

      if (is_some()) {
        return Returned{mem::move(*this).unwrap_unchecked(unsafe)};
      }

      return Returned{std::invoke(mem::forward<F>(mapper))};
    }

    /// @copydoc Option::or_else
    template<ty::provider F>
    requires implicit_copy_allowed
    [[nodiscard]] CRAB_INLINE constexpr auto or_else(F&& mapper) const& {
      return copied().or_else(mem::forward<F>(mapper));
    }

    /// If this option is of some type Option<Option<T>>, this will flatten
    /// it to a single Option<T>
    [[nodiscard]] CRAB_INLINE constexpr auto flatten() && -> T requires option_type<T>
    {
      if (is_none()) {
        return None{};
      }

      return mem::move(*this).unwrap_unchecked(unsafe);
    }

    /// Implicit Copy Overload.
    /// @copydoc Option::flatten
    [[nodiscard]] CRAB_INLINE constexpr auto flatten() const& -> T requires option_type<T> and implicit_copy_allowed

    {
      return copied().flatten();
    }

    /// Consumes this option, if this is some and passes the predicate it
    /// will return Some, else None
    template<ty::predicate<const T&> F>
    [[nodiscard]] CRAB_INLINE constexpr auto filter(F&& predicate) && -> Option {
      if (is_none()) {
        return {};
      }

      T value{mem::move(*this).unwrap_unchecked(unsafe)};

      const bool passed{
        static_cast<bool>(std::invoke(mem::forward<F>(predicate), implicit_cast<const T&>(value))),
      };

      if (not passed) {
        return {};
      }

      return Option{mem::forward<Contained>(value)};
    }

    /// Implicit copy overload.
    /// @copydoc Option::filter
    template<ty::predicate<const T&> F>
    requires implicit_copy_allowed
    [[nodiscard]] CRAB_INLINE constexpr auto filter(F&& predicate) const& -> Option<T> {
      return copied().filter(mem::forward<F>(predicate));
    }

    /// Combines many Option values into one if they are all Some,
    /// if any of the options are empty, this returns an empty option.
    ///
    /// This method does not support implicit copies, if needed manually call Option::copied.
    ///
    /// # Example
    /// ```cpp
    /// void print_sum(Option<i32> a, Option<i32> b) {
    ///  a.zip(b);
    /// }
    /// ````
    template<typename... Vals>
    [[nodiscard]] CRAB_INLINE constexpr auto zip(Option<Vals>... other) && -> Option<Tuple<T, Vals...>> {
      if (is_none() or (... or other.is_none())) {
        return None{};
      }

      return {
        Tuple<T, Vals...>{mem::move(*this).unwrap_unchecked(unsafe), mem::move(other).unwrap_unchecked(unsafe)...}
      };
    }

    /// }@

    /// Checks if this result contains a value, and if so does it also
    /// match with the given predicate
    ///
    /// `Option<i32>{10}.is_ok_and(crab::fn::even) -> true`
    /// `Option<i32>{10}.is_ok_and(crab::fn::odd) -> false`
    template<ty::predicate<const T&> F>
    [[nodiscard]] CRAB_INLINE constexpr auto is_some_and(F&& functor) const -> bool {
      return is_some() and static_cast<bool>(std::invoke(mem::forward<F>(functor), get_unchecked(unsafe)));
    }

    /// Boolean 'and' operation on two options, this will return none if either of
    /// the two options are none, and if both are some this will return the
    /// contained value of the second
    template<typename U = T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator and(Option<U> other) && -> Option<U> {
      if (is_none()) {
        return None{};
      }

      return other;
    }

    /// Boolean 'and' operation on two options, this will return none if either of
    /// the two options are none, and if both are some this will return the
    /// contained value of the second
    template<typename U = T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator and(Option<U> other) const& -> Option<U>
      requires implicit_copy_allowed
    {
      return copied() and mem::move(other);
    }

    /// Boolean or operation, if this option is some it will return that, if the
    /// other option is some it will return that, else this returns none
    [[nodiscard]] CRAB_INLINE constexpr auto operator or(Option other) && -> Option {
      if (is_none()) {
        return other;
      }

      return mem::move(*this);
    }

    /// @copydoc Option::operator or
    [[nodiscard]] CRAB_INLINE constexpr auto operator or(Option other) const& -> Option requires implicit_copy_allowed
    {
      return copied() or mem::move(other);
    }

    /// The same as or but if both options are some then this will be none
    [[nodiscard]] CRAB_INLINE constexpr auto operator xor(Option other) && -> Option {
      if (is_none()) {
        return other;
      }

      if (other.is_some()) {
        return None{};
      }

      return mem::move(*this);
    }

    /// @copydoc Option::operator xor
    [[nodiscard]] CRAB_INLINE constexpr auto operator xor(Option other) const& -> Option requires implicit_copy_allowed
    {
      return copied() xor mem::move(other);
    }

    /// TODO: documnet
    template<typename U = T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator==(const Option<U>& other) const -> bool {
      static_assert(
        std::equality_comparable_with<T, U>,
        "Cannot equate to options if the inner types are not equatable"
      );
      if (is_none() or other.is_none()) {
        return is_none() == other.is_none();
      }

      return static_cast<bool>(get_unchecked(unsafe) == other.get_unchecked(unsafe));
    }

    template<typename U = T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator!=(const Option<U>& other) const -> bool {
      static_assert(
        std::equality_comparable_with<T, U>,
        "Cannot equate to options if the inner types are not inverse equatable"
      );

      if (is_none() or other.is_none()) {
        return is_none() != other.is_none();
      }

      return static_cast<bool>(get_unchecked(unsafe) != other.get_unchecked(unsafe));
    }

    template<typename U = T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator>(const Option<U>& other) const -> bool {
      static_assert(
        requires(const T& a, const U& b) {
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

      return static_cast<bool>(get_unchecked(unsafe) < other.get_unchecked(unsafe));
    }

    /// TODO: documnet
    template<typename U = T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator>=(const Option<U>& other) const -> bool {
      static_assert(
        requires(const T& a, const U& b) {
          { a >= b } -> ty::convertible<bool>;
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

      return static_cast<bool>(get_unchecked(unsafe) >= other.get_unchecked(unsafe));
    }

    /// TODO: documnet
    template<typename U = T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator<=(const Option<U>& other) const -> bool {
      static_assert(
        requires(const T& a, const U& b) {
          { a <= b } -> ty::convertible<bool>;
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

      return static_cast<bool>(get_unchecked(unsafe) <= other.get_unchecked(unsafe));
    }

    /// TODO: documnet
    template<typename U = T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator<=>(const Option<U>& other) const -> std::partial_ordering {
      static_assert(
        std::three_way_comparable_with<T, U>,
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

      return static_cast<std::partial_ordering>(get_unchecked(unsafe) <=> other.get_unchecked(unsafe));
    }

    /// Option<T> == None only if the former is none
    [[nodiscard]] CRAB_INLINE constexpr auto operator==(const None&) const -> bool {
      return is_none();
    }

    /// Option<T> != None only if the former is some
    [[nodiscard]] CRAB_INLINE constexpr auto operator!=(const None&) const -> bool {
      return is_some();
    }

    /// Option<T> > None only if the former is some
    [[nodiscard]] CRAB_INLINE constexpr auto operator>(const None&) const -> bool {
      return is_some();
    }

    /// Option<T> >= None is always true
    [[nodiscard]] CRAB_INLINE constexpr auto operator>=(const None&) const -> bool {
      return true;
    }

    /// Option<T> < None is never true
    [[nodiscard]] CRAB_INLINE constexpr auto operator<(const None&) const -> bool {
      return false;
    }

    /// Option<T> <= None is never true
    [[nodiscard]] CRAB_INLINE constexpr auto operator<=(const None&) const -> bool {
      return is_none();
    }

  private:

    /// Storage for the contained type, this is not stored directly but instead uses an auxillary struct dependent on
    /// the contained type to support niche specialisations.
    /// @internal
    Storage storage;
  };
}

/// Formatter specialization for Option<T>, this specialization is only valid if the inner type of T is formattable
/// itself.
/// @ingroup opt
template<fmt::formattable T, typename Char>
struct fmt::formatter<crab::opt::Option<T>, Char> {
private:

  inline static constexpr StringView some{"Some("};
  inline static constexpr StringView none{"None"};

  formatter<std::remove_cv_t<T>, Char> underlying;

public:

  constexpr auto parse(parse_context<Char>& ctx) {
    detail::maybe_set_debug_format(underlying, true);
    return underlying.parse(ctx);
  }

  template<typename FormatContext>
  constexpr auto format(const crab::opt::Option<T>& opt, FormatContext& ctx) const -> decltype(ctx.out()) {
    if (opt.is_none()) {
      return detail::write<Char>(ctx.out(), none);
    }

    auto out = ctx.out();

    // write some(
    out = detail::write<Char>(out, some);

    ctx.advance_to(out);

    out = underlying.format(opt.get_unchecked(unsafe), ctx);

    // write trailing )
    return detail::write(out, ')');
  }
};

/// Hasher specialization for Option<T>, this specialisation is only valid of the inner type T is hashable itself.
/// @ingroup opt
template<crab::ty::hashable T>
struct std::hash<::crab::opt::Option<T>> /* NOLINT */ {
  /// Converts the given option 'opt' into a hash
  /// @param opt Option to hash
  /// @internal
  [[nodiscard]] CRAB_INLINE constexpr auto operator()(const ::crab::opt::Option<T>& opt) const -> crab::hash_code {
    if (opt.is_none()) {
      return crab::hash(false);
    }

    return crab::hash_code_mix(true, opt.get_unchecked(unsafe));
  }
};

namespace crab::prelude {
  using opt::Option;
}

CRAB_PRELUDE_GUARD;

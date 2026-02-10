/// @file crab/result/Result.hpp
/// @ingroup result

// Created by bishan_ on 4/23/24.
//

// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <concepts>
#include <type_traits>

#include "crab/any/AnyOf.hpp"
#include "crab/assertion/check.hpp"
#include "crab/core/unsafe.hpp"
#include "crab/result/concepts.hpp"
#include "crab/mem/forward.hpp"
#include "crab/opt/Option.hpp"
#include "crab/result/Ok.hpp"
#include "crab/result/Err.hpp"
#include "crab/result/Error.hpp"

namespace crab::result {

  // NOLINTBEGIN(*explicit*)

  /// A type used to encapsulate a return type that could possible be an error instead.
  /// @ingroup result
  template<typename T, typename E>
  class Result final {

    /// A type T must be a valid 'Ok<T>' type
    static_assert(ok_type<T>, "T is not a valid Ok type, must be move constructible.");

    /// A type E must be a valid 'Err<T>' type
    static_assert(error_type<E>, "E is not a valid Err type.");

  public:

    /// Alias for the container result:Err instantiated with this result's error type
    using Err = result::Err<E>;

    /// Alias for the container result:Ok instantiated with this result's 'ok' type
    using Ok = result::Ok<T>;

    /// "Err" type for this result, Result<T, E>::ErrType = E
    using ErrType = E;

    /// "Ok" type for this result, Result<T, E>::OkType = T
    using OkType = T;

    /// Concept for if this results 'ok' and 'err' types are the same / similar, meaning this will consider them the
    /// 'same' if they are different reference types but the same fundamental (ex. T& vs const T& vs volatile T&).
    inline static constexpr bool is_same{
      std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<E>>,
    };

    /// Concept for if this result is copyable
    inline static constexpr bool is_copyable{std::copyable<T> and std::copyable<E>};

    /// Concept for if this result is
    inline static constexpr bool implicit_copy_allowed{
      std::is_trivially_copyable_v<T> and std::is_trivially_copyable_v<E>,
    };

  private:

    /// Type for the internal storage of this result.
    /// @internal
    using AnyOf = any::AnyOf<Ok, Err>;

  public:

    /// Copy construction from an Ok type. This constructor is not available if the type is not copyable, is a reference
    /// type, or if this result's ok and error types are the same. If this constructor is unvailable, consider
    /// Result::Result(T&&) or Result::Result(Ok)
    CRAB_INLINE constexpr Result(const T& from) requires(not is_same and std::copyable<T> and not ty::is_reference<T>)
        : Result{T(from)} {}

    /// Copy construction from an Error type. This constructor is not available if the type is not copyable, is a
    /// reference type, or if this result's ok and error types are the same. If this constructor is unvailable, consider
    /// Result::Result(E&&) or Result::Result(Err)
    CRAB_INLINE constexpr Result(const E& from) requires(not is_same and std::copyable<E> and not ty::is_reference<T>)
        : Result{E(from)} {}

    /// Move / perfect forward constructor for an Ok type. This method is unavailable if this results error and ok types
    /// are the same - in which case the only way to construct one would be to use the explicit 'Ok' tag type with
    /// Result::Result(Ok)
    CRAB_INLINE constexpr Result(T&& from) requires(not is_same)
        : Result{Ok{mem::forward<T>(from)}} {}

    /// Move / perfect forward constructor for an Err type. This method is unavailable if this results error and ok
    /// types are the same - in which case the only way to construct one would be to use the explicit 'Err' tag type
    /// with Result::Result(Err)
    CRAB_INLINE constexpr Result(E&& from) requires(not is_same)
        : Result{Err{mem::forward<E>(from)}} {}

    /// Constructs a result with the given 'Ok' value. This constructor is always available no matter the ok or error
    /// type.
    CRAB_INLINE constexpr Result(Ok from): storage{AnyOf::template from<Ok>(mem::move(from))} {}

    /// Constructs a result with the given 'Err value. This constructor is always available no matter the ok or error
    CRAB_INLINE constexpr Result(Err from): storage{AnyOf::template from<Err>(mem::move(from))} {}

    /// Reassigns this option to the given 'Ok' value
    CRAB_INLINE constexpr auto operator=(Ok from) -> Result& {
      storage.template emplace<Ok>(mem::move<Ok>(from));
      return *this;
    }

    /// Reassigns this option to the given 'Err' value
    CRAB_INLINE constexpr auto operator=(Err from) -> Result& {
      storage.template emplace<Err>(mem::move<Err>(from));
      return *this;
    }

    /// Reassigns this given option the the ok value, this method is not available if this result's error & result
    /// fundamental types are the same, in which case you should use Result::operator=(Ok) instead.
    CRAB_INLINE constexpr auto operator=(T&& from) -> Result& requires(not is_same)
    {
      storage.template emplace<Ok>(mem::forward<T>(from));
      return *this;
    }

    /// Reassigns this given option the the err value, this method is not available if this result's error & result
    /// fundamental types are the same, in which case you should use Result::operator=(Err) instead.
    CRAB_INLINE constexpr auto operator=(E&& from) -> Result& requires(not is_same)
    {
      storage.template emplace<Err>(mem::forward<E>(from));
      return *this;
    }

    /// *Explicit* conversion operator between a result and a bool, this is not intended to be used directly and should
    /// normally be replaced with is_ok, this operator exists mainly to support 'if init' syntax.
    ///
    /// This operator behaves the same as is_ok and inherits its panic states.
    ///
    /// # Examples
    ///
    /// ```cpp
    /// void parse_i32(StringView str) -> Result<i32, ParseError>;
    ///
    /// if (Result<i32, ParseError> result = parse_i32("15")) {
    ///   fmt::println("Num: {}", result.get());
    /// }
    /// ```
    [[nodiscard]] CRAB_INLINE constexpr explicit operator bool() const {
      return is_ok();
    }

    /// Returns true if this result holds an 'Ok' value.
    ///
    /// # Panics
    /// This will panic if called on a moved-from result, if you wish to store a possibly empty Result you should use
    /// Option<Result<T>>
    [[nodiscard]] CRAB_INLINE constexpr auto is_ok() const -> bool {
      check_unmoved();
      return is_ok_unchecked(unsafe);
    }

    /// Returns true if this result holds an 'Ok' value. This is meant to be used in contexts where you know for sure
    /// you are working with a not moved-from result, however in most cases you should simply use
    /// [is_ok](#Result::is_ok)
    [[nodiscard]] CRAB_INLINE constexpr auto is_ok_unchecked(unsafe_fn) const -> bool {
      return storage.template is_unchecked<Ok>(unsafe);
    }

    /// Returns true of this option holds an 'Err' value
    ///
    /// # Panics
    /// This will panic if called on a moved-from result, if you wish to store a possibly empty Result you should use
    /// Option<Result<T>>
    [[nodiscard]] CRAB_INLINE constexpr auto is_err() const -> bool {
      check_unmoved();
      return is_err_unchecked(unsafe);
    }

    /// Returns true if this result holds an 'Err' value. This is meant to be used in contexts where you know for sure
    /// you are working with a not moved-from result, however in most cases you should simply use
    /// [is_err](#Result::is_err)
    [[nodiscard]] CRAB_INLINE constexpr auto is_err_unchecked(unsafe_fn) const -> bool {
      return storage.template is_unchecked<Err>(unsafe);
    }

    /// Checks if this result contains an Ok value, and if so does it also
    /// match with the given predicate.
    ///
    ///
    /// # Examples
    ///
    /// ```cpp
    /// auto is_even = [](i32 x) {  return x != 0; };
    /// Result<i32,String>{10}.is_ok_and(crab::fn::even);
    /// Result<i32,String>{10}.is_ok_and(crab::fn::odd);
    /// ```
    template<crab::ty::predicate<const T&> F>
    [[nodiscard]] CRAB_INLINE constexpr auto is_ok_and(F&& functor) const -> bool {
      if (not is_ok()) {
        return false;
      }

      return std::invoke(std::forward<F>(functor), get_unchecked(unsafe));
    }

    /// Checks if this result contains an Err value, and if so does it also
    /// match with the given predicate
    template<crab::ty::predicate<const E&> F>
    [[nodiscard]] CRAB_INLINE constexpr auto is_err_and(F&& functor) const -> bool {
      if (not is_err()) {
        return false;
      }

      return std::invoke(std::forward<F>(functor), get_err_unchecked(unsafe));
    }

    /// TODO: doc
    [[nodiscard]] auto get(const SourceLocation loc = SourceLocation::current()) & -> T& {
      check_is_ok(loc);
      return storage.template as_unchecked<Ok>(unsafe).get();
    }

    /// Const variant.
    /// @copydoc get
    [[nodiscard]] auto get(const SourceLocation loc = SourceLocation::current()) const& -> const T& {
      check_is_ok(loc);
      return storage.template as_unchecked<Ok>(unsafe).get();
    }

    /// TODO: doc
    [[nodiscard]] auto get_err(const SourceLocation loc = SourceLocation::current()) & -> E& {
      check_is_err(loc);
      return storage.template as_unchecked<Err>(unsafe).get();
    }

    /// Const variant.
    /// @copydoc get_err
    [[nodiscard]] auto get_err(const SourceLocation loc = SourceLocation::current()) const& -> const E& {
      check_is_err(loc);
      return storage.template as_unchecked<Err>(unsafe).get();
    }

    /// Gets a reference to the contained inner Ok value, if there is no Ok
    /// value this will panic and crash.
    [[nodiscard]] CRAB_INLINE constexpr auto get_unchecked(
      unsafe_fn,
      const SourceLocation loc = SourceLocation::current()
    ) & -> T& {
      check_dbg_is_ok(loc);
      return storage.template as_unchecked<Ok>(unsafe).get();
    }

    /// Const variant of get_unchecked.
    /// @copydoc get_unchecked
    [[nodiscard]] CRAB_INLINE constexpr auto get_unchecked(
      unsafe_fn,
      const SourceLocation loc = SourceLocation::current()
    ) const& -> const T& {
      check_dbg_is_ok(loc);
      return storage.template as_unchecked<Ok>(unsafe).get();
    }

    /// Gets a reference to the contained Error value, if there is no Error
    /// value this will panic and crash.
    [[nodiscard]] CRAB_INLINE constexpr auto get_err_unchecked(
      unsafe_fn,
      const SourceLocation loc = SourceLocation::current()
    ) & -> E& {
      check_dbg_is_err(loc);
      return storage.template as_unchecked<Err>(unsafe).get();
    }

    /// Const variant of get_unchecked.
    /// @copydoc get_err_unchecked
    [[nodiscard]] CRAB_INLINE constexpr auto get_err_unchecked(
      unsafe_fn,
      const SourceLocation loc = SourceLocation::current()
    ) const& -> const E& {
      check_dbg_is_err(loc);
      return storage.template as_unchecked<Err>(unsafe).get();
    }

    [[nodiscard]] CRAB_INLINE constexpr auto unwrap(const SourceLocation loc = SourceLocation::current()) && -> T {
      check_is_ok(loc);
      return mem::move(storage).template as_unchecked<Ok>(unsafe).get();
    }

    [[nodiscard]] CRAB_INLINE constexpr auto unwrap_unchecked(
      unsafe_fn,
      const SourceLocation loc = SourceLocation::current()
    ) && -> T {
      check_is_ok(loc);
      return mem::move(storage).template as_unchecked<Ok>(unsafe).get();
    }

    [[nodiscard]] CRAB_INLINE constexpr auto unwrap_err(const SourceLocation loc = SourceLocation::current()) && -> E {
      check_is_err(loc);
      return mem::move(storage).template as_unchecked<Err>(unsafe).get();
    }

    [[nodiscard]] CRAB_INLINE constexpr auto unwrap_err_unchecked(
      unsafe_fn,
      const SourceLocation loc = SourceLocation::current()
    ) && -> E {
      check_is_err(loc);
      return mem::move(storage).template as_unchecked<Err>(unsafe).get();
    }

    friend CRAB_INLINE constexpr auto operator<<(std::ostream& os, const Result& result) -> std::ostream& {
      result.check_unmoved();

      if (result.is_err()) {
        return os << "Err(" << error_to_string(result.get_err_unchecked(unsafe)) << ")";
      }

      return os << "Ok(" << result.get_unchecked(unsafe) << ")";
    }

    /// Copies this option, this is an alias for simply calling the copy constructor. It suggested to use this explicit
    /// copy when possible as it can make the intention clearer.
    ///
    /// # Panics
    /// This will panic if the underlying result has been moved
    [[nodiscard]]
    CRAB_INLINE constexpr auto copied() const -> Result requires is_copyable
    {
      if (is_ok()) {
        return {OkType(get_unchecked(unsafe))};
      }

      return {ErrType(get_err_unchecked(unsafe))};
    }

    /// @name Monadic Operations
    /// @{

    template<typename Into>
    [[nodiscard]] CRAB_INLINE constexpr auto map() && -> Result<Into, E> {
      static_assert(
        crab::ty::convertible<T, Into>,
        "'Result<T, E>::map<Into>()' can only be done if T is convertible to Into"
      );

      return mem::move(*this).map([](T&& value) -> Into { return static_cast<Into>(mem::forward<T>(value)); });
    }

    template<typename Into>
    requires is_copyable
    [[nodiscard]] CRAB_INLINE constexpr auto map() const& -> Result<Into, E> {
      return copied().template map<Into>();
    }

    template<typename Into>
    [[nodiscard]] CRAB_INLINE constexpr auto map_err() && -> Result<T, Into> {
      static_assert(
        crab::ty::convertible<E, Into>,
        "'Result<T, E>::map<Into>()' can only be done if E is convertible to Into"
      );

      return mem::move(*this).map_err([](E&& value) -> Into { return static_cast<Into>(mem::forward<E>(value)); });
    }

    template<typename Into>
    requires is_copyable
    [[nodiscard]] CRAB_INLINE constexpr auto map_err() const& -> Result<T, E> {
      return copied().template map_err<Into>();
    }

    /// Consumes self and if not Error, maps the Ok value to a new value
    /// @tparam F
    template<crab::ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto map(F&& functor) && {
      using R = ty::functor_result<F, T>;

      if (is_ok()) {
        return Result<R, E>{
          result::Ok<R>{std::invoke(mem::forward<F>(functor), mem::move(*this).unwrap_unchecked(unsafe))}
        };
      }

      return Result<R, E>{result::Err<E>{mem::move(*this).unwrap_err_unchecked(unsafe)}};
    }

    /// Consumes self and if not Ok, maps the Error value to a new value
    /// @tparam F
    template<crab::ty::mapper<E> F>
    [[nodiscard]] CRAB_INLINE constexpr auto map_err(F&& functor) && {
      using R = ty::functor_result<F, E>;

      if (is_err()) {
        return Result<T, R>{
          result::Err<R>{std::invoke(functor, mem::move(*this).unwrap_err_unchecked(unsafe))},
        };
      }

      return Result<T, R>{result::Ok<T>{mem::move(*this).unwrap_unchecked(unsafe)}};
    }

    /// Consumes self and if not Error, maps the Ok value to a new value
    /// @tparam F
    template<crab::ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto map(F&& functor) const& {
      static_assert(
        implicit_copy_allowed,
        "Only results with trivial Ok & Err types may be implicitly copied when "
        "using monadic operations, you must call Result::copied() yourself."
      );

      return copied().map(mem::forward<F>(functor));
    }

    /**
     * Consumes self and if not Ok, maps the Error value to a new value
     * @tparam F
     * @return
     */
    template<crab::ty::mapper<E> F>
    [[nodiscard]] CRAB_INLINE constexpr auto map_err(F&& functor) const& {
      static_assert(
        implicit_copy_allowed,
        "Only results with trivial Ok & Err types may be implicitly copied when "
        "using monadic operations, you must call Result::copied() yourself."
      );

      return copied().map_err(mem::forward<F>(functor));
    }

    // ========================= flat map functions ==============================

    /// Monadic function that consumes / moves the value in this option.
    /// If result is Ok, run function on the ok value,
    /// If the mapped function is Ok(type M), it returns Result<M, Error>
    template<crab::ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto and_then(F&& functor) && {
      using R = ty::functor_result<F, T>;

      static_assert(result_type<R>, "and_then functor parameter must return a Result<T,E> type");

      static_assert(
        std::same_as<typename R::ErrType, ErrType>,
        "Returned result must have the same error as the original monadic "
        "context (must be a mapping from Result<T,E> -> Result<S,E> ) "
      );

      if (is_err()) {
        return R{Err{mem::move(*this).unwrap_err_unchecked(unsafe)}};
      }

      return std::invoke(functor, mem::move(*this).unwrap_unchecked(unsafe));
    }

    /// Mondadic function that copies the inner value. If result is Ok, run
    /// function on the ok value, If the mapped function is Ok(type M), it returns
    /// Result<M, Error>
    template<crab::ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto and_then(F&& functor) const& requires implicit_copy_allowed
    {
      return copied().and_then(mem::forward<F>(functor));
    }

    /// Takes Ok value out of this object and returns it, if is Err and not
    /// Ok, an empty option will be returned instead.
    ///
    /// This function is for use when wanting to abstract away any specific error
    /// type to simply 'none'.
    [[nodiscard]] CRAB_INLINE constexpr auto into_ok() && -> opt::Option<T> {

      if (is_ok()) {
        return {mem::move(*this).unwrap_unchecked(unsafe)};
      }
      return {};
    }

    /// Takes Ok value out of this object and returns it, if is Err and not
    /// Ok, an empty option will be returned instead.
    ///
    /// This function is for use when wanting to abstract away any specific error
    /// type to simply 'none'.
    [[nodiscard]] CRAB_INLINE constexpr auto into_ok() const& -> opt::Option<T> requires implicit_copy_allowed
    {
      return copied().into_ok();
    }

    /// Takes Err value out of this object and returns it, if is Ok and not
    /// Err, an empty option will be returned instead.
    ///
    /// This function is for use when wanting to abstract away any specific Ok type
    /// to simply 'none'.
    [[nodiscard]] CRAB_INLINE constexpr auto into_err() && -> opt::Option<E> {
      if (is_err()) {
        return {mem::move(*this).unwrap_err_unchecked(unsafe)};
      }

      return {};
    }

    /// Takes Err value out of this object and returns it, if is Ok and not
    /// Err, an empty option will be returned instead.
    ///
    /// This function is for use when wanting to abstract away any specific Ok type
    /// to simply 'none'.
    [[nodiscard]] CRAB_INLINE constexpr auto into_err() const& -> opt::Option<E> requires implicit_copy_allowed
    {
      return copied().into_err();
    }

    /// TODO: doc
    [[nodiscard]] auto is_valid() const -> bool {
      return storage.is_valid();
    }

    /// }@

  private:

    /// Internal method for preventing use-after-movas
    CRAB_INLINE constexpr auto check_unmoved(const SourceLocation loc = SourceLocation::current()) const -> void {
      crab_check_with_location(storage.is_valid(), loc, "Invalid use of moved result");
    }

    CRAB_INLINE constexpr auto check_dbg_unmoved(const SourceLocation loc = SourceLocation::current()) const -> void {
      discard(loc);
#if CRAB_DEBUG
      check_unmoved(loc);
#endif
    }

    CRAB_INLINE constexpr auto check_is_ok(const SourceLocation loc = SourceLocation::current()) const -> void {
      check_unmoved();

      crab_check_with_location(
        is_ok(),
        loc,
        "Excepted result to contain an 'Ok' value, instead found error: {}",
        error_to_string(get_err_unchecked(unsafe))
      );
    }

    CRAB_INLINE constexpr auto check_dbg_is_ok(const SourceLocation loc = SourceLocation::current()) const -> void {
      discard(loc);
#if CRAB_DEBUG
      check_is_ok(loc);
#endif
    }

    CRAB_INLINE constexpr auto check_is_err(const SourceLocation loc = SourceLocation::current()) const -> void {
      check_unmoved();

      crab_check_with_location(
        is_err(),
        loc,
        "Excepted result to contain an 'Err' value, instead found an 'Ok' value."
      );
    }

    CRAB_INLINE constexpr auto check_dbg_is_err(const SourceLocation loc = SourceLocation::current()) const -> void {
      discard(loc);
#if CRAB_DEBUG
      check_is_err(loc);
#endif
    }

    /// @internal
    /// Internal storage for any Result, this uses a plain AnyOf
    AnyOf storage;
  };

  // NOLINTEND(*explicit*)

} // namespace crab::result

namespace crab::prelude {
  using result::Result;
}

CRAB_PRELUDE_GUARD;

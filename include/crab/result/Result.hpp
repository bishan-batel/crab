//

// Created by bishan_ on 4/23/24.
//

// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <concepts>
#include <type_traits>

#include "crab/any/AnyOf.hpp"
#include "crab/assertion/check.hpp"
#include "crab/result/concepts.hpp"
#include "crab/mem/forward.hpp"
#include "crab/opt/Option.hpp"
#include "crab/result/Ok.hpp"
#include "crab/result/Err.hpp"
#include "crab/result/Error.hpp"

namespace crab::result {

  // NOLINTBEGIN(*explicit*)

  template<typename T, typename E>
  class Result final {
    static_assert(ok_type<T>, "T is not a valid Ok type, must be move constructible.");
    static_assert(error_type<E>, "E is not a valid Err type.");

  public:

    using Err = result::Err<E>;
    using Ok = result::Ok<T>;

    /**
     * "Err" type for this result, Result<T, E>::ErrType = E
     */
    using ErrType = E;

    /**
     * "Ok" type for this result, Result<T, E>::OkType = T
     */
    using OkType = T;

    inline static constexpr bool is_same{std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<E>>};

    inline static constexpr bool is_copyable{std::copyable<T> and std::copyable<E>};

    inline static constexpr bool is_trivially_copyable{
      std::is_trivially_copyable_v<T> and std::is_trivially_copyable_v<E>,
    };

  private:

    using AnyOf = any::AnyOf<Ok, Err>;
    AnyOf storage;

  public:

    CRAB_INLINE constexpr Result(const T& from) requires(not is_same and std::copyable<T>)
        : Result{T(from)} {}

    CRAB_INLINE constexpr Result(const E& from) requires(not is_same and std::copyable<E>)
        : Result{E(from)} {}

    CRAB_INLINE constexpr Result(T&& from) requires(not is_same)
        : Result{Ok{mem::forward<T>(from)}} {}

    CRAB_INLINE constexpr Result(E&& from) requires(not is_same)
        : Result{Err{mem::forward<E>(from)}} {}

    CRAB_INLINE constexpr Result(Ok&& from): storage{AnyOf::template from<Ok>(mem::move(from))} {}

    CRAB_INLINE constexpr Result(Err&& from): storage{AnyOf::template from<Err>(mem::move(from))} {}

    CRAB_INLINE constexpr Result(Result&& from) noexcept: storage{mem::move(from.storage)} {}

    CRAB_INLINE constexpr Result(const Result& res): storage{res.storage} {
      static_assert(is_copyable, "Cannot copy a result with a non-copyable Err or Ok type");
    }

    CRAB_INLINE constexpr auto operator=(const Result& res) -> Result& {
      static_assert(is_copyable, "cannot copy a result with a non-copyable err or ok type");
      if (&res == this) {
        return *this;
      }

      storage = res.storage;

      return *this;
    }

    CRAB_INLINE constexpr auto operator=(Result&& res) noexcept -> Result& {
      storage = mem::move(res.storage);
      return *this;
    }

    CRAB_INLINE constexpr auto operator=(Ok from) -> Result& {
      storage.template emplace<Ok>(mem::move<Ok>(from));
      return *this;
    }

    CRAB_INLINE constexpr auto operator=(Err from) -> Result& {
      storage.template emplace<Err>(mem::move<Err>(from));
      return *this;
    }

    CRAB_INLINE constexpr auto operator=(T&& from) -> Result& requires(not is_same)
    {
      storage.template emplace<Ok>(mem::forward<T>(from));
      return *this;
    }

    CRAB_INLINE constexpr auto operator=(E&& from) -> Result& requires(not is_same)
    {
      storage.template emplace<Err>(mem::forward<E>(from));
      return *this;
    }

    [[nodiscard]] CRAB_INLINE constexpr explicit operator bool() const {
      return is_ok();
    }

    /**
     * @brief Does this result hold an ok value
     */
    [[nodiscard]] CRAB_INLINE constexpr auto is_ok() const -> bool {
      return storage.template is<Ok>();
    }

    /**
     * @brief Does this result hold an err value
     */
    [[nodiscard]] CRAB_INLINE constexpr auto is_err() const -> bool {
      return storage.template is<Err>();
    }

    /**
     * @brief Checks if this result contains an Ok value, and if so does it also
     * match with the given predicate
     *
     * Result<i32,String>{10}.is_ok_and(crab::fn::even) -> true
     * Result<i32,String>{10}.is_ok_and(crab::fn::odd) -> false
     */
    template<crab::ty::predicate<const T&> F>
    [[nodiscard]] CRAB_INLINE constexpr auto is_ok_and(F&& functor) const -> bool {
      return is_ok() and std::invoke(functor, get_unchecked());
    }

    /**
     * @brief Checks if this result contains an Err value, and if so does it also
     * match with the given predicate
     */
    template<crab::ty::predicate<const E&> F>
    [[nodiscard]] CRAB_INLINE constexpr auto is_err_and(F&& functor) const -> bool {
      return is_err() and std::invoke(functor, get_err_unchecked());
    }

    /**
     * @brief Gets a reference to the contained inner Ok value, if there is no Ok
     * value this will panic and crash.
     */
    [[nodiscard]] CRAB_INLINE constexpr auto get_unchecked(const SourceLocation loc = SourceLocation::current()) -> T& {
      ensure_valid(loc);

      crab_check_with_location(
        is_ok(),
        loc,
        "Called unwrap on result with Error:\n{}",
        error_to_string(get_err_unchecked())
      );

      return storage.template as<Ok>().unwrap().get();
    }

    /**
     * @brief Gets a reference to the contained Error value, if there is no Error
     * value this will panic and crash.
     */
    [[nodiscard]] CRAB_INLINE constexpr auto get_err_unchecked(const SourceLocation loc = SourceLocation::current())
      -> E& {
      ensure_valid(loc);

      crab_check_with_location(is_err(), loc, "Called unwrap with Ok value");

      return storage.template as<Err>().unwrap().get();
    }

    /**
     * @brief Gets a reference to the contained inner Ok value, if there is no Ok
     * value this will panic and crash.
     */
    [[nodiscard]] CRAB_INLINE constexpr auto get_unchecked(const SourceLocation loc = SourceLocation::current()) const
      -> const T& {
      ensure_valid(loc);

      crab_check_with_location(
        is_ok(),
        loc,
        "Called unwrap on result with Error:\n{}",
        error_to_string(get_err_unchecked())
      );

      return storage.template as<Ok>().unwrap().get();
    }

    /**
     * @brief Gets a reference to the contained Error value, if there is no Error
     * value this will panic and crash.
     */
    [[nodiscard]] CRAB_INLINE constexpr auto get_err_unchecked(
      const SourceLocation loc = SourceLocation::current()
    ) const -> const E& {
      ensure_valid(loc);

      crab_check_with_location(is_err(), loc, "Called unwrap on Ok value");

      return storage.template as<Err>().unwrap().get();
    }

    [[nodiscard]] CRAB_INLINE constexpr auto unwrap(const SourceLocation loc = SourceLocation::current()) && -> T {
      ensure_valid(loc);

      crab_check_with_location(
        is_ok(),
        loc,
        "Called unwrap on result with Error:\n{}",
        error_to_string(get_err_unchecked())
      );

      return mem::move(storage).template as<Ok>().unwrap().get();
    }

    [[nodiscard]] CRAB_INLINE constexpr auto unwrap_err(const SourceLocation loc = SourceLocation::current()) && -> E {
      ensure_valid(loc);

      crab_check_with_location(is_err(), loc, "Called unwrap_err on result with Ok value");

      return mem::move(storage).template as<Err>().unwrap().get();
    }

    /**
     * @brief Internal method for preventing use-after-movas
     */
    CRAB_INLINE constexpr auto ensure_valid(const SourceLocation loc = SourceLocation::current()) const -> void {
      crab_check_with_location(storage.is_valid(), loc, "Invalid use of moved result");
    }

    friend CRAB_INLINE constexpr auto operator<<(std::ostream& os, const Result& result) -> std::ostream& {
      if (result.is_err()) {
        return os << "Err(" << error_to_string(result.get_err_unchecked()) << ")";
      }

      return os << "Ok(" << result.get_unchecked() << ")";
    }

    [[nodiscard]] CRAB_INLINE constexpr auto copied(const SourceLocation loc = SourceLocation::current()) const
      -> Result {
      static_assert(is_copyable, "Cannot copy a result whose Ok and Err types are not both copyable");

      ensure_valid(loc);

      return is_ok() ? Result{OkType{get_unchecked()}} : Result{ErrType{get_err_unchecked()}};
    }

    // ===========================================================================
    //                            Monadic Operations
    // ===========================================================================

    // ============================ map functions ================================
    //
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

    /**
     * Consumes self and if not Error, maps the Ok value to a new value
     * @tparam F
     * @return
     */
    template<crab::ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto map(F&& functor, const SourceLocation loc = SourceLocation::current()) && {
      using R = ty::functor_result<F, T>;

      ensure_valid(loc);

      if (is_ok()) {
        return Result<R, E>{result::Ok<R>{std::invoke(functor, mem::move(*this).unwrap(loc))}};
      }

      return Result<R, E>{result::Err<E>{mem::move(*this).unwrap_err(loc)}};
    }

    /**
     * Consumes self and if not Ok, maps the Error value to a new value
     * @tparam F
     * @return
     */
    template<crab::ty::mapper<E> F>
    [[nodiscard]] CRAB_INLINE constexpr auto map_err(
      F&& functor,
      const SourceLocation loc = SourceLocation::current()
    ) && {
      using R = ty::functor_result<F, E>;

      ensure_valid(loc);

      if (is_err()) {
        return Result<T, R>{
          result::Err<R>{std::invoke(functor, mem::move(*this).unwrap_err(loc))},
        };
      }

      return Result<T, R>{result::Ok<T>{mem::move(*this).unwrap(loc)}};
    }

    /**
     * Consumes self and if not Error, maps the Ok value to a new value
     * @tparam F
     * @return
     */
    template<crab::ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto map(
      F&& functor,
      const SourceLocation loc = SourceLocation::current()
    ) const& {
      static_assert(
        is_trivially_copyable,
        "Only results with trivial Ok & Err types may be implicitly copied when "
        "using monadic operations, you must call Result::copied() yourself."
      );

      return copied(loc).map(mem::forward<F>(functor), loc);
    }

    /**
     * Consumes self and if not Ok, maps the Error value to a new value
     * @tparam F
     * @return
     */
    template<crab::ty::mapper<E> F>
    [[nodiscard]] CRAB_INLINE constexpr auto map_err(
      F&& functor,
      const SourceLocation loc = SourceLocation::current()
    ) const& {
      static_assert(
        is_trivially_copyable,
        "Only results with trivial Ok & Err types may be implicitly copied when "
        "using monadic operations, you must call Result::copied() yourself."
      );

      return copied(loc).map_err(mem::forward<F>(functor), loc);
    }

    // ========================= flat map functions ==============================

    /**
     * @brief Monadic function that consumes / moves the value in this option.
     * If result is Ok, run function on the ok value,
     * If the mapped function is Ok(type M), it returns Result<M, Error>
     */
    template<crab::ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto and_then(
      F&& functor,
      const SourceLocation loc = SourceLocation::current()
    ) && {
      using R = ty::functor_result<F, T>;

      static_assert(result_type<R>, "and_then functor parameter must return a Result<T,E> type");

      static_assert(
        std::same_as<typename R::ErrType, ErrType>,
        "Returned result must have the same error as the original monadic "
        "context (must be a mapping from Result<T,E> -> Result<S,E> ) "
      );

      ensure_valid(loc);

      if (is_err()) {
        return R{Err{mem::move(*this).unwrap_err(loc)}};
      }

      return std::invoke(functor, mem::move(*this).unwrap(loc));
    }

    /**
     * @brief Mondadic function that copies the inner value. If result is Ok, run
     * function on the ok value, If the mapped function is Ok(type M), it returns
     * Result<M, Error>
     */
    template<crab::ty::mapper<T> F>
    [[nodiscard]] CRAB_INLINE constexpr auto and_then(
      F&& functor,
      const SourceLocation loc = SourceLocation::current()
    ) const& {

      static_assert(
        is_trivially_copyable,
        "Only results with trivial Ok & Err types may be implicitly copied when "
        "using monadic operations, you must call Result::copied() yourself."
      );

      return copied(loc).and_then(mem::forward<F>(functor));
    }

    /**
     * @brief Takes Ok value out of this object and returns it, if is Err and not
     * Ok, an empty option will be returned instead.
     *
     * This function is for use when wanting to abstract away any specific error
     * type to simply 'none'.
     */
    [[nodiscard]] CRAB_INLINE constexpr auto ok(
      const SourceLocation loc = SourceLocation::current()
    ) && -> opt::Option<T> {
      ensure_valid(loc);

      return is_ok() ? opt::Option<T>{mem::move(*this).unwrap(loc)} : opt::Option<T>{};
    }

    /**
     * @brief Takes Err value out of this object and returns it, if is Ok and not
     * Err, an empty option will be returned instead.
     *
     * This function is for use when wanting to abstract away any specific Ok type
     * to simply 'none'.
     */
    [[nodiscard]] CRAB_INLINE constexpr auto err(
      const SourceLocation loc = SourceLocation::current()
    ) && -> opt::Option<E> {
      ensure_valid(loc);

      return is_err() ? opt::Option<E>{mem::move(*this).unwrap_err(loc)} : opt::Option<E>{};
    }

    /**
     * @brief Takes Ok value out of this object and returns it, if is Err and not
     * Ok, an empty option will be returned instead.
     *
     * This function is for use when wanting to abstract away any specific error
     * type to simply 'none'.
     */
    [[nodiscard]] CRAB_INLINE constexpr auto ok(
      const SourceLocation loc = SourceLocation::current()
    ) const& -> opt::Option<T> {
      static_assert(
        std::is_trivially_copyable_v<T>,
        "Only results with trivial Ok types may be implicitly copied when "
        "using reductive monadic operations, you must call Result::copied() "
        "yourself."
      );

      return copied().ok(loc);
    }

    /**
     * @brief Takes Err value out of this object and returns it, if is Ok and not
     * Err, an empty option will be returned instead.
     *
     * This function is for use when wanting to abstract away any specific Ok type
     * to simply 'none'.
     */
    [[nodiscard]] CRAB_INLINE constexpr auto err(
      const SourceLocation loc = SourceLocation::current()
    ) const& -> opt::Option<E> {
      static_assert(
        std::is_trivially_copyable_v<E>,
        "Only results with trivial Ok types may be implicitly copied when "
        "using reductive monadic operations, you must call Result::copied() "
        "yourself."
      );

      return copied().err(loc);
    }
  };

  // NOLINTEND(*explicit*)

} // namespace crab::result

namespace crab::prelude {
  using result::Result;
}

CRAB_PRELUDE_GUARD;

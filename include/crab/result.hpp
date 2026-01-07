//

// Created by bishan_ on 4/23/24.
//

// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <concepts>
#include <format>
#include <type_traits>
#include <variant>

#include "./preamble.hpp"
#include "./fmt.hpp"
#include "./debug.hpp"
#include "./type_traits.hpp"
#include "./option.hpp"

namespace crab {
  /**
   * @brief Base error type for use with Result<T, E>
   */
  class Error {
  public:

    constexpr Error() = default;

    constexpr Error(const Error&) = default;

    constexpr Error(Error&&) = default;

    constexpr Error& operator=(const Error&) = default;

    constexpr Error& operator=(Error&&) = default;

    constexpr virtual ~Error() = default;

    /**
     * @brief Converts this crab Error into a runtime exception that can be
     * thrown, if needed when dealing with certain API's
     *
     * @return
     */
    CRAB_NODISCARD auto as_exception() const -> std::runtime_error {
      return std::runtime_error{what()};
    }

    /**
     * @brief Stringified error message for logging purposes
     */
    CRAB_NODISCARD virtual auto what() const -> String = 0;
  };

  namespace result {

    /**
     * @brief Converts a given error to its stringified representation.
     */
    template<ty::error_type E>
    CRAB_NODISCARD_CONSTEXPR auto error_to_string(const E& err) {
      if constexpr (requires {
                      { err.what() } -> crab::ty::convertible<String>;
                    }) {
        return err.what();
      }

      else if constexpr (requires {
                           { err->what() } -> crab::ty::convertible<String>;
                         }) {
        return err->what();
      }

      else if constexpr (requires { OutStringStream{} << err; }) {
        return (OutStringStream{} << err).str();
      }

      else if constexpr (std::is_enum_v<E>) {
        return crab::format("{}[{}]", typeid(E).name(), static_cast<std::underlying_type_t<E>>(err));
      } else {
        return typeid(E).name();
      }
    }

    /**
     * @brief Thin wrapper over a value to be given to Result<T,E>(Ok)'s
     * constructor
     */
    template<typename T>
    struct Ok {
      static_assert(ty::ok_type<T>, "Ok<T> must satisfy ok_type");

      using Inner = T;
      T value;

      CRAB_INLINE_CONSTEXPR explicit Ok(T value): value(std::move(value)) {}
    };

    template<typename T>
    struct Ok<T&> : Ok<std::reference_wrapper<T>> {
      CRAB_INLINE_CONSTEXPR explicit Ok(T& value): Ok<std::reference_wrapper<T>>{std::reference_wrapper<T>{value}} {}
    };

    /**
     * @brief Thin wrapper over a value to be given to Result<T,E>(Err)'s
     * constructor
     */
    template<typename E>
    struct Err {
      static_assert(ty::error_type<E>, "Err<E> must satisfy ok_type");

      using Inner = E;

      E value;

      CRAB_INLINE_CONSTEXPR explicit Err(E value): value(std::move(value)) {}
    };

    template<typename T>
    struct Err<T&> : Err<std::reference_wrapper<T>> {
      CRAB_INLINE_CONSTEXPR explicit Err(T& value): Err<std::reference_wrapper<T>>{std::reference_wrapper<T>{value}} {}
    };

    namespace impl {
      /**
       * @brief Whether type T is the 'Ok' wrapper Ok<K>
       */
      template<typename>
      struct is_crab_ok final : std::false_type {};

      /**
       * @brief Whether type T is the 'Ok' wrapper Ok<K>
       */
      template<ty::ok_type T>
      struct is_crab_ok<Ok<T>> final : std::true_type {};

      /**
       * @brief Whether type T is the 'Err' wrapper Err<K>
       */
      template<typename>
      struct is_crab_err final : std::false_type {};

      /**
       * @brief Whether type T is the 'Err' wrapper Err<K>
       */
      template<ty::error_type E>
      struct is_crab_err<Err<E>> final : std::true_type {};
    }

    /**
     * Type predicate for if the given type is of the form crab::Ok<T>
     */
    template<typename T>
    concept crab_ok = impl::is_crab_ok<T>::value;

    /**
     * Type predicate for if the given type is of the form crab::Err<T>
     */
    template<typename T>
    concept crab_err = impl::is_crab_err<T>::value;

    // NOLINTBEGIN(*explicit*)

    template<typename T, typename E>
    class Result final {
      static_assert(ty::ok_type<T>, "T is not a valid Ok type, must be move constructible.");
      static_assert(ty::error_type<E>, "E is not a valid Err type.");

    public:

      using Err = ::crab::result::Err<E>;
      using Ok = ::crab::result::Ok<T>;

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

      struct invalidated final {};

      std::variant<Ok, Err, invalidated> inner;

    public:

      CRAB_INLINE_CONSTEXPR Result(const T& from) requires(not is_same and std::copyable<T>)
          : Result{T{from}} {}

      CRAB_INLINE_CONSTEXPR Result(const E& from) requires(not is_same and std::copyable<E>)
          : Result{E{from}} {}

      CRAB_INLINE_CONSTEXPR Result(T&& from) requires(not is_same)
          : Result{Ok{std::forward<T>(from)}} {}

      CRAB_INLINE_CONSTEXPR Result(E&& from) requires(not is_same)
          : Result{Err{std::forward<E>(from)}} {}

      CRAB_INLINE_CONSTEXPR Result(Ok&& from): inner{std::forward<Ok>(from)} {}

      CRAB_INLINE_CONSTEXPR Result(Err&& from): inner{std::forward<Err>(from)} {}

      CRAB_INLINE_CONSTEXPR Result(Result&& from) noexcept: inner{std::exchange(from.inner, invalidated{})} {}

      CRAB_INLINE_CONSTEXPR Result(const Result& res): inner{res.inner} {
        static_assert(is_copyable, "Cannot copy a result with a non-copyable Err or Ok type");
      }

      CRAB_INLINE_CONSTEXPR auto operator=(const Result& res) -> Result& {
        static_assert(is_copyable, "cannot copy a result with a non-copyable err or ok type");
        if (&res == this) {
          return *this;
        }

        inner = res.inner;

        return *this;
      }

      CRAB_INLINE_CONSTEXPR auto operator=(Result&& res) noexcept -> Result& {
        inner = std::exchange(res.inner, invalidated{});
        return *this;
      }

      CRAB_INLINE_CONSTEXPR auto operator=(Ok&& from) -> Result& {
        inner = std::forward<Ok>(from);
        return *this;
      }

      CRAB_INLINE_CONSTEXPR auto operator=(Err&& from) -> Result& {
        inner = std::forward<Err>(from);
        return *this;
      }

      CRAB_INLINE_CONSTEXPR auto operator=(T&& from) -> Result& requires(not is_same)
      {
        return *this = Ok{std::forward<T>(from)}; /* NOLINT(*operator*)*/
      }

      CRAB_INLINE_CONSTEXPR auto operator=(E&& from) -> Result& requires(not is_same)
      {
        return *this = Err{std::forward<E>(from)}; /* NOLINT(*operator*)*/
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR explicit operator bool() const {
        return is_ok();
      }

      /**
       * @brief Does this result hold an ok value
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto is_ok() const -> bool {
        return std::holds_alternative<Ok>(inner);
      }

      /**
       * @brief Does this result hold an err value
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto is_err() const -> bool {
        return std::holds_alternative<Err>(inner);
      }

      /**
       * @brief Checks if this result contains an Ok value, and if so does it also
       * match with the given predicate
       *
       * Result<i32,String>{10}.is_ok_and(crab::fn::even) -> true
       * Result<i32,String>{10}.is_ok_and(crab::fn::odd) -> false
       */
      template<crab::ty::predicate<const T&> F>
      CRAB_NODISCARD_INLINE_CONSTEXPR auto is_ok_and(F&& functor) const -> bool {
        return is_ok() and std::invoke(functor, get_unchecked());
      }

      /**
       * @brief Checks if this result contains an Err value, and if so does it also
       * match with the given predicate
       */
      template<crab::ty::predicate<const E&> F>
      CRAB_NODISCARD_INLINE_CONSTEXPR auto is_err_and(F&& functor) const -> bool {
        return is_err() and std::invoke(functor, get_err_unchecked());
      }

      /**
       * @brief Gets a reference to the contained inner Ok value, if there is no Ok
       * value this will panic and crash.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto get_unchecked(const SourceLocation loc = SourceLocation::current()) -> T& {
        ensure_valid(loc);

        debug_assert_transparent(
          is_ok(),
          loc,
          "Called unwrap on result with Error:\n{}",
          ::crab::result::error_to_string(get_err_unchecked())
        );

        return std::get<Ok>(inner).value;
      }

      /**
       * @brief Gets a reference to the contained Error value, if there is no Error
       * value this will panic and crash.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto get_err_unchecked(const SourceLocation loc = SourceLocation::current()) -> E& {
        ensure_valid(loc);

        debug_assert_transparent(is_err(), loc, "Called unwrap with Ok value");

        return std::get<Err>(inner).value;
      }

      /**
       * @brief Gets a reference to the contained inner Ok value, if there is no Ok
       * value this will panic and crash.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto get_unchecked(const SourceLocation loc = SourceLocation::current()) const
        -> const T& {
        ensure_valid(loc);

        debug_assert_transparent(
          is_ok(),
          loc,
          "Called unwrap on result with Error:\n{}",
          ::crab::result::error_to_string(get_err_unchecked())
        );

        return std::get<Ok>(inner).value;
      }

      /**
       * @brief Gets a reference to the contained Error value, if there is no Error
       * value this will panic and crash.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto get_err_unchecked(const SourceLocation loc = SourceLocation::current()) const
        -> const E& {
        ensure_valid(loc);

        debug_assert_transparent(is_err(), loc, "Called unwrap on Ok value");

        return std::get<Err>(inner).value;
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto unwrap(const SourceLocation loc = SourceLocation::current()) && -> T {
        ensure_valid(loc);

        debug_assert_transparent(
          is_ok(),
          loc,
          "Called unwrap on result with Error:\n{}",
          ::crab::result::error_to_string(get_err_unchecked())
        );

        return std::get<Ok>(std::exchange(inner, invalidated{})).value;
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto unwrap_err(const SourceLocation loc = SourceLocation::current()) && -> E {
        ensure_valid(loc);

        debug_assert_transparent(is_err(), loc, "Called unwrap_err on result with Ok value");

        return std::get<Err>(std::exchange(inner, invalidated{})).value;
      }

      /**
       * @brief Internal method for preventing use-after-movas
       */
      CRAB_INLINE_CONSTEXPR auto ensure_valid(const SourceLocation loc = SourceLocation::current()) const -> void {

        debug_assert_transparent(not std::holds_alternative<invalidated>(inner), loc, "Invalid use of moved result");
      }

      friend CRAB_INLINE_CONSTEXPR auto operator<<(std::ostream& os, const Result& result) -> std::ostream& {
        if (result.is_err()) {
          return os << "Err(" << ::crab::result::error_to_string(result.get_err_unchecked()) << ")";
        }

        return os << "Ok(" << result.get_unchecked() << ")";
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto copied(const SourceLocation loc = SourceLocation::current()) const -> Result {
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
      CRAB_NODISCARD_INLINE_CONSTEXPR auto map() && -> Result<Into, E> {
        static_assert(
          crab::ty::convertible<T, Into>,
          "'Result<T, E>::map<Into>()' can only be done if T is convertible to Into"
        );

        return std::move(*this).map([](T&& value) -> Into { return static_cast<Into>(std::forward<T>(value)); });
      }

      template<typename Into>
      requires is_copyable
      CRAB_NODISCARD_INLINE_CONSTEXPR auto map() const& -> Result<Into, E> {
        return copied().template map<Into>();
      }

      template<typename Into>
      CRAB_NODISCARD_INLINE_CONSTEXPR auto map_err() && -> Result<T, Into> {
        static_assert(
          crab::ty::convertible<E, Into>,
          "'Result<T, E>::map<Into>()' can only be done if E is convertible to Into"
        );

        return std::move(*this).map_err([](E&& value) -> Into { return static_cast<Into>(std::forward<E>(value)); });
      }

      template<typename Into>
      requires is_copyable
      CRAB_NODISCARD_INLINE_CONSTEXPR auto map_err() const& -> Result<T, E> {
        return copied().template map_err<Into>();
      }

      /**
       * Consumes self and if not Error, maps the Ok value to a new value
       * @tparam F
       * @return
       */
      template<crab::ty::mapper<T> F>
      CRAB_NODISCARD_INLINE_CONSTEXPR auto map(F&& functor, const SourceLocation loc = SourceLocation::current()) && {
        using R = ty::functor_result<F, T>;

        ensure_valid(loc);

        if (is_ok()) {
          return Result<R, E>{result::Ok<R>{std::invoke(functor, std::move(*this).unwrap(loc))}};
        }

        return Result<R, E>{result::Err<E>{std::move(*this).unwrap_err(loc)}};
      }

      /**
       * Consumes self and if not Ok, maps the Error value to a new value
       * @tparam F
       * @return
       */
      template<crab::ty::mapper<E> F>
      CRAB_NODISCARD_INLINE_CONSTEXPR auto map_err(F&& functor, const SourceLocation loc = SourceLocation::current()) && {
        using R = ty::functor_result<F, E>;

        ensure_valid(loc);

        if (is_err()) {
          return Result<T, R>{
            result::Err<R>{std::invoke(functor, std::move(*this).unwrap_err(loc))},
          };
        }

        return Result<T, R>{result::Ok<T>{std::move(*this).unwrap(loc)}};
      }

      /**
       * Consumes self and if not Error, maps the Ok value to a new value
       * @tparam F
       * @return
       */
      template<crab::ty::mapper<T> F>
      CRAB_NODISCARD_INLINE_CONSTEXPR auto map(F&& functor, const SourceLocation loc = SourceLocation::current()) const& {
        static_assert(
          is_trivially_copyable,
          "Only results with trivial Ok & Err types may be implicitly copied when "
          "using monadic operations, you must call Result::copied() yourself."
        );

        return copied(loc).map(std::forward<F>(functor), loc);
      }

      /**
       * Consumes self and if not Ok, maps the Error value to a new value
       * @tparam F
       * @return
       */
      template<crab::ty::mapper<E> F>
      CRAB_NODISCARD_INLINE_CONSTEXPR auto map_err(
        F&& functor,
        const SourceLocation loc = SourceLocation::current()
      ) const& {
        static_assert(
          is_trivially_copyable,
          "Only results with trivial Ok & Err types may be implicitly copied when "
          "using monadic operations, you must call Result::copied() yourself."
        );

        return copied(loc).map_err(std::forward<F>(functor), loc);
      }

      // ========================= flat map functions ==============================

      /**
       * @brief Monadic function that consumes / moves the value in this option.
       * If result is Ok, run function on the ok value,
       * If the mapped function is Ok(type M), it returns Result<M, Error>
       */
      template<crab::ty::mapper<T> F>
      CRAB_NODISCARD_INLINE_CONSTEXPR auto and_then(F&& functor, const SourceLocation loc = SourceLocation::current()) && {
        using R = ty::functor_result<F, T>;

        static_assert(ty::result_type<R>, "and_then functor parameter must return a Result<T,E> type");

        static_assert(
          std::same_as<typename R::ErrType, ErrType>,
          "Returned result must have the same error as the original monadic "
          "context (must be a mapping from Result<T,E> -> Result<S,E> ) "
        );

        ensure_valid(loc);

        if (is_err()) {
          return R{Err{std::move(*this).unwrap_err(loc)}};
        }

        return std::invoke(functor, std::move(*this).unwrap(loc));
      }

      /**
       * @brief Mondadic function that copies the inner value. If result is Ok, run
       * function on the ok value, If the mapped function is Ok(type M), it returns
       * Result<M, Error>
       */
      template<crab::ty::mapper<T> F>
      CRAB_NODISCARD_INLINE_CONSTEXPR auto and_then(
        F&& functor,
        const SourceLocation loc = SourceLocation::current()
      ) const& {

        static_assert(
          is_trivially_copyable,
          "Only results with trivial Ok & Err types may be implicitly copied when "
          "using monadic operations, you must call Result::copied() yourself."
        );

        return copied(loc).and_then(std::forward<F>(functor));
      }

      /**
       * @brief Takes Ok value out of this object and returns it, if is Err and not
       * Ok, an empty option will be returned instead.
       *
       * This function is for use when wanting to abstract away any specific error
       * type to simply 'none'.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto ok(const SourceLocation loc = SourceLocation::current()) && -> Option<T> {
        ensure_valid(loc);

        return is_ok() ? Option<T>{std::move(*this).unwrap(loc)} : Option<T>{};
      }

      /**
       * @brief Takes Err value out of this object and returns it, if is Ok and not
       * Err, an empty option will be returned instead.
       *
       * This function is for use when wanting to abstract away any specific Ok type
       * to simply 'none'.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto err(const SourceLocation loc = SourceLocation::current()) && -> Option<E> {
        ensure_valid(loc);

        return is_err() ? Option<E>{std::move(*this).unwrap_err(loc)} : Option<E>{};
      }

      /**
       * @brief Takes Ok value out of this object and returns it, if is Err and not
       * Ok, an empty option will be returned instead.
       *
       * This function is for use when wanting to abstract away any specific error
       * type to simply 'none'.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto ok(const SourceLocation loc = SourceLocation::current()) const& -> Option<T> {
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
      CRAB_NODISCARD_INLINE_CONSTEXPR auto err(const SourceLocation loc = SourceLocation::current()) const& -> Option<E> {
        static_assert(
          std::is_trivially_copyable_v<E>,
          "Only results with trivial Ok types may be implicitly copied when "
          "using reductive monadic operations, you must call Result::copied() "
          "yourself."
        );

        return copied().err(loc);
      }
    };
  }

  // NOLINTEND(*explicit*)

  namespace result {

    namespace impl {
      template<ty::error_type Error>
      struct fallible {

        template<typename... T>
        CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(Tuple<T...> tuple) const {
          return Result<Tuple<T...>, Error>{std::forward<Tuple<T...>>(tuple)};
        }

        template<typename PrevResults, ty::provider F, ty::provider... Rest>
        requires ty::result_type<ty::functor_result<F>>
        CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(
          PrevResults tuple /* Tuple<T...>*/,
          F&& function,
          Rest&&... other_functions
        ) const {
          return std::invoke(function).and_then([&]<typename R>(R&& result) {
            return operator()(
              std::tuple_cat(std::move(tuple), Tuple<R>(std::forward<R>(result))),
              std::forward<Rest>(other_functions)...
            );
          });
        }

        template<typename PrevResults, std::invocable F, std::invocable... Rest>
        requires(ty::option_type<std::invoke_result_t<F>> and std::is_default_constructible_v<Error>)
        CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(
          PrevResults tuple /* Tuple<T...>*/,
          F&& function,
          Rest&&... other_functions
        ) const {
          return std::invoke(function).template ok_or<Error>(
                                        []() -> Error { return Error{}; }
          ).flat_map([&]<typename R>(R&& result) {
            return operator()(
              std::tuple_cat(std::move(tuple), Tuple<R>(std::forward<R>(result))),
              std::forward<Rest>(other_functions)...
            );
          });
        }

        template<typename PrevResults, std::invocable F, std::invocable... Rest>
        requires(not ty::result_type<std::invoke_result_t<F>>)
        CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(
          PrevResults tuple /* Tuple<T...>*/,
          F&& function,
          Rest&&... other_functions
        ) const {
          return operator()(
            std::tuple_cat(std::move(tuple), Tuple<std::invoke_result_t<F>>(std::invoke(function))),
            std::forward<Rest>(other_functions)...
          );
        }

        template<typename PrevResults, typename V, typename... Rest>
        requires(not std::invocable<V> and not ty::result_type<V>)
        CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(
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
        CRAB_NODISCARD_INLINE_CONSTEXPR auto operator()(
          PrevResults tuple, /* Tuple<T...>*/
          Result<V, Error> value,
          Rest&&... other_functions
        ) const {
          return std::move(value).and_then([&]<typename R>(R&& result) {
            return operator()(
              std::tuple_cat(std::move(tuple), Tuple<R>(std::forward<R>(result))),
              std::forward<Rest>(other_functions)...
            );
          });
        }
      };
    }

    template<ty::error_type E, std::invocable... F>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto fallible(F&&... fallible) {
      return impl::fallible<E>{}(Tuple<>{}, std::forward<F>(fallible)...);
    }

    template<ty::ok_type T, typename... Args>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto ok(Args&&... args) {
      return Ok<T>{T{std::forward<Args>(args)...}};
    }

    template<ty::error_type E, typename... Args>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto err(Args&&... args) {
      return Err<E>{E{std::forward<Args>(args)...}};
    }

    template<typename T>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto ok(T value) {
      static_assert(ty::ok_type<T>, "Value must be a possible 'Ok<T>' type for use in Result<T, E>");

      return Ok<T>{std::move(value)};
    }

    template<typename E>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto err(E value) {

      static_assert(ty::ok_type<E>, "Value must be a possible 'Err<E>' type for use in Result<T, E>");

      return Err<E>{std::move(value)};
    }

    template<typename T, typename E>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto unwrap(Result<T, E>&& result, const SourceLocation loc = SourceLocation::current())
      -> T {
      return std::forward<Result<T, E>>(result).unwrap(loc);
    }

    template<typename T, typename E>
    CRAB_NODISCARD_INLINE_CONSTEXPR auto unwrap_err(
      Result<T, E>&& result,
      const SourceLocation loc = SourceLocation::current()
    ) -> E {
      return std::forward<Result<T, E>>(result).unwrap_err(loc);
    }

  } // namespace result

  using result::Ok;
  using result::Err;
  using result::fallible;
  using result::unwrap_err;
  using result::ok;
  using result::err;
  using result::unwrap;
} // namespace crab

#if CRAB_USE_PRELUDE
using ::crab::result::Result;
#endif

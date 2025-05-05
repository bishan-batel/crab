//

// Created by bishan_ on 4/23/24.
//

// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <concepts>
#include <format>
#include <sstream>
#include <type_traits>
#include <variant>

#include "crab/debug.hpp"
#include "crab/type_traits.hpp"
#include "option.hpp"

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
     * @brief Stringified error message for logging purposes
     */
    [[nodiscard]] constexpr virtual auto what() const -> String = 0;
  };
} // namespace crab

namespace crab::result {
  /**
   * @brief A valid error type.
   */
  template<typename E> concept error_type = std::move_constructible<E>;

  /**
   * @brief Converts a given error to its stringified representation.
   */
  template<error_type E>
  [[nodiscard]] constexpr auto error_to_string(const E& err) {
    if constexpr (requires {
                    { err.what() } -> std::convertible_to<String>;
                  }) {
      return err.what();
    }

    else if constexpr (requires {
                         { err->what() } -> std::convertible_to<String>;
                       }) {
      return err->what();
    }

    else if constexpr (requires { OutStringStream{} << err; }) {
      return (OutStringStream{} << err).str();
    }

    else if constexpr (std::is_enum_v<E>) {
      return std::format(
        "{}[{}]",
        typeid(E).name(),
        static_cast<std::underlying_type_t<E>>(err)
      );
    } else {
      return typeid(E).name();
    }
  }

  /**
   * @brief Type constraint for a type that can be used with Result<T>
   */
  template<typename T> concept ok_type = std::move_constructible<T>;

  /**
   * @brief Thin wrapper over a value to be given to Result<T,E>(Ok)'s
   * constructor
   */
  template<typename T>
  struct Ok {
    static_assert(ok_type<T>, "Ok<T> must satisfy ok_type");

    using Inner = T;
    T value;

    constexpr explicit Ok(T value): value(std::move(value)) {}
  };

  template<typename T>
  struct Ok<T&> : Ok<std::reference_wrapper<T>> {
    constexpr explicit Ok(T& value):
        Ok<std::reference_wrapper<T>>{std::reference_wrapper<T>{value}} {}
  };

  /**
   * @brief Thin wrapper over a value to be given to Result<T,E>(Err)'s
   * constructor
   */
  template<typename E>
  struct Err {
    static_assert(error_type<E>, "Err<E> must satisfy ok_type");

    using Inner = E;

    E value;

    constexpr explicit Err(E value): value(std::move(value)) {}
  };

  template<typename T>
  struct Err<T&> : Err<std::reference_wrapper<T>> {
    constexpr explicit Err(T& value):
        Err<std::reference_wrapper<T>>{std::reference_wrapper<T>{value}} {}
  };

  template<typename>
  struct is_result_type {
    static constexpr bool value = false;
  };

  template<typename T, typename E>
  struct is_result_type<Result<T, E>> {
    static constexpr bool value = true;
  };

  /**
   * @brief Whether type T is a Result<>
   */
  template<typename T>
  constexpr bool is_result_type_v = is_result_type<T>::value;

  /**
   * @brief Whether type T is the 'Ok' wrapper Ok<K>
   */
  template<typename>
  struct is_ok_type {
    static constexpr bool value = false;
  };

  /**
   * @brief Whether type T is the 'Ok' wrapper Ok<K>
   */
  template<ok_type T>
  struct is_ok_type<Ok<T>> {
    static constexpr bool value = true;
  };

  /**
   * @brief Whether type T is the 'Err' wrapper Err<K>
   */
  template<typename>
  struct is_err_type {
    static constexpr bool value = false;
  };

  /**
   * @brief Whether type T is the 'Err' wrapper Err<K>
   */
  template<error_type E>
  struct is_err_type<Err<E>> {
    static constexpr bool value = true;
  };

} // namespace crab::result

// NOLINTBEGIN(*explicit*)

template<typename T, typename E>
class Result final {
  static_assert(
    crab::result::ok_type<T>,
    "T is not a valid Ok type, must be move constructible."
  );
  static_assert(crab::result::error_type<E>, "E is not a valid Err type.");
};

template<crab::result::ok_type T, crab::result::error_type E>
class Result<T, E> final {
public:

  using Err = crab::result::Err<E>;
  using Ok = crab::result::Ok<T>;

  /**
   * "Err" type for this result, Result<T, E>::ErrType = E
   */
  using ErrType = E;

  /**
   * "Ok" type for this result, Result<T, E>::OkType = T
   */
  using OkType = T;

  static constexpr bool is_same =
    std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<E>>;

  static constexpr bool is_copyable = std::copyable<T> and std::copyable<E>;

  static constexpr bool is_trivially_copyable =
    std::is_trivially_copyable_v<T> and std::is_trivially_copyable_v<E>;

private:

  struct invalidated {};

  std::variant<Ok, Err, invalidated> inner;

public:

  // constexpr Result(const T& from) requires(not is_same)
  //     : Result{Ok{from}} {
  //   static_assert(
  //     std::copyable<T> and not is_same,
  //     "Cannot create result with const ref ok type with a non-copyable ok
  //     type"
  //   );
  // }

  // constexpr Result(const E& from) requires(not is_same)
  //     : Result{Err{from}} {
  //   static_assert(
  //     std::copyable<E> and not is_same,
  //     "Cannot create result with const ref err type with a non-copyable err "
  //     "type"
  //   );
  // }

  constexpr Result(T&& from) requires(not is_same)
      : Result{Ok{std::forward<T>(from)}} {}

  constexpr Result(E&& from) requires(not is_same)
      : Result{Err{std::forward<E>(from)}} {}

  constexpr Result(Ok&& from): inner{std::forward<Ok>(from)} {}

  constexpr Result(Err&& from): inner{std::forward<Err>(from)} {}

  constexpr Result(Result&& from) noexcept:
      inner{std::exchange(from.inner, invalidated{})} {}

  constexpr Result(const Result& res): inner{res.inner} {
    static_assert(
      is_copyable,
      "Cannot copy a result with a non-copyable Err or Ok type"
    );
  }

  constexpr auto operator=(const Result& res) -> Result& {
    static_assert(
      is_copyable,
      "cannot copy a result with a non-copyable err or ok type"
    );
    if (&res == this) {
      return *this;
    }

    inner = res.inner;

    return *this;
  }

  constexpr auto operator=(Result&& res) noexcept -> Result& {
    inner = std::exchange(res.inner, invalidated{});
    return *this;
  }

  constexpr auto operator=(Ok&& from) -> Result& {
    inner = std::forward<Ok>(from);
    return *this;
  }

  constexpr auto operator=(Err&& from) -> Result& {
    inner = std::forward<Err>(from);
    return *this;
  }

  constexpr auto operator=(T&& from) -> Result& requires(not is_same)
  {
    return *this = Ok{std::forward<T>(from)}; /* NOLINT(*operator*)*/
  }

  constexpr auto operator=(E&& from) -> Result& requires(not is_same)
  {
    return *this = Err{std::forward<E>(from)}; /* NOLINT(*operator*)*/
  }

  [[nodiscard]] constexpr explicit operator bool() const { return is_ok(); }

  /**
   * @brief Does this result hold an ok value
   */
  [[nodiscard]] constexpr auto is_ok() const -> bool {
    return std::holds_alternative<Ok>(inner);
  }

  /**
   * @brief Does this result hold an err value
   */
  [[nodiscard]] constexpr auto is_err() const -> bool {
    return std::holds_alternative<Err>(inner);
  }

  /**
   * @brief Checks if this result contains an Ok value, and if so does it also
   * match with the given predicate
   *
   * Result<i32,String>{10}.is_ok_and(crab::fn::even) -> true
   * Result<i32,String>{10}.is_ok_and(crab::fn::odd) -> false
   */
  template<std::predicate<const T&> F>
  [[nodiscard]] constexpr auto is_ok_and(F&& functor) const -> bool {
    return is_ok() and std::invoke(functor, get_unchecked());
  }

  /**
   * @brief Checks if this result contains an Err value, and if so does it also
   * match with the given predicate
   */
  template<std::predicate<const E&> F>
  [[nodiscard]] constexpr auto is_err_and(F&& functor) const -> bool {
    return is_err() and std::invoke(functor, get_err_unchecked());
  }

  /**
   * @brief Gets a reference to the contained inner Ok value, if there is no Ok
   * value this will panic and crash.
   */
  [[nodiscard]] constexpr auto get_unchecked(
    const std::source_location loc = std::source_location::current()
  ) -> T& {
    ensure_valid(loc);
    debug_assert_transparent(
      is_ok(),
      std::format(
        "Called unwrap on result with Error:\n{}",
        crab::result::error_to_string(get_err_unchecked())
      ),
      loc
    );
    return std::get<Ok>(inner).value;
  }

  /**
   * @brief Gets a reference to the contained Error value, if there is no Error
   * value this will panic and crash.
   */
  [[nodiscard]] constexpr auto get_err_unchecked(
    const std::source_location loc = std::source_location::current()
  ) -> E& {
    ensure_valid(loc);
    debug_assert_transparent(is_err(), "Called unwrap with Ok value", loc);
    return std::get<Err>(inner).value;
  }

  /**
   * @brief Gets a reference to the contained inner Ok value, if there is no Ok
   * value this will panic and crash.
   */
  [[nodiscard]] constexpr auto get_unchecked(
    const std::source_location loc = std::source_location::current()
  ) const -> const T& {
    ensure_valid(loc);
    debug_assert_transparent(
      is_ok(),
      std::format(
        "Called unwrap on result with Error:\n{}",
        crab::result::error_to_string(get_err_unchecked())
      ),
      loc
    );
    return std::get<Ok>(inner).value;
  }

  /**
   * @brief Gets a reference to the contained Error value, if there is no Error
   * value this will panic and crash.
   */
  [[nodiscard]] constexpr auto get_err_unchecked(
    const std::source_location loc = std::source_location::current()
  ) const -> const E& {
    ensure_valid(loc);
    debug_assert_transparent(is_err(), "Called unwrap on Ok value", loc);
    return std::get<Err>(inner).value;
  }

  [[nodiscard]] constexpr auto unwrap(
    const std::source_location loc = std::source_location::current()
  ) && -> T {
    ensure_valid(loc);
    debug_assert_transparent(
      is_ok(),
      std::format(
        "Called unwrap on result with Error:\n{}",
        crab::result::error_to_string(get_err_unchecked())
      ),
      loc
    );
    return std::get<Ok>(std::exchange(inner, invalidated{})).value;
  }

  [[nodiscard]] constexpr auto unwrap_err(
    const std::source_location loc = std::source_location::current()
  ) && -> E {
    ensure_valid(loc);
    debug_assert_transparent(
      is_err(),
      "Called unwrap_err on result with Ok value",
      loc
    );
    return std::get<Err>(std::exchange(inner, invalidated{})).value;
  }

  /**
   * @brief Internal method for preventing use-after-movas
   */
  constexpr auto ensure_valid(
    const std::source_location loc = std::source_location::current()
  ) const -> void {
    debug_assert_transparent(
      not std::holds_alternative<invalidated>(inner),
      "Invalid use of moved result",
      loc
    );
  }

  friend constexpr auto operator<<(std::ostream& os, const Result& result)
    -> std::ostream& {
    if (result.is_err()) {
      return os << "Err(" << result.get_err_unchecked() << ")";
    }
    return os << "Ok(" << result.get_unchecked() << ")";
  }

  [[nodiscard]] constexpr auto copied(
    const std::source_location loc = std::source_location::current()
  ) const -> Result {
    static_assert(
      is_copyable,
      "Cannot copy a result whose Ok and Err types are not both copyable"
    );

    ensure_valid(loc);

    return is_ok() ? Result{OkType{get_unchecked()}}
                   : Result{ErrType{get_err_unchecked()}};
  }

  // ===========================================================================
  //                            Monadic Operations
  // ===========================================================================

  // ============================ map functions ================================

  /**
   * Consumes self and if not Error, maps the Ok value to a new value
   * @tparam F
   * @return
   */
  template<std::invocable<T> F>
  [[nodiscard]] constexpr auto map(
    F&& functor,
    const std::source_location loc = std::source_location::current()
  ) && {
    using R = std::invoke_result_t<F, T>;

    ensure_valid(loc);

    if (is_ok()) {
      return Result<R, E>{
        crab::result::Ok<R>{std::invoke(functor, std::move(*this).unwrap(loc))}
      };
    }

    return Result<R, E>{crab::result::Err<E>{std::move(*this).unwrap_err(loc)}};
  }

  /**
   * Consumes self and if not Ok, maps the Error value to a new value
   * @tparam F
   * @return
   */
  template<std::invocable<E> F>
  [[nodiscard]] constexpr auto map_err(
    F&& functor,
    const std::source_location loc = std::source_location::current()
  ) && {
    using R = std::invoke_result_t<F, E>;

    ensure_valid(loc);

    if (is_err()) {
      return Result<T, R>{
        crab::result::Err<R>{
          std::invoke(functor, std::move(*this).unwrap_err(loc))
        },
      };
    }

    return Result<T, R>{crab::result::Ok<T>{std::move(*this).unwrap(loc)}};
  }

  /**
   * Consumes self and if not Error, maps the Ok value to a new value
   * @tparam F
   * @return
   */
  template<std::invocable<T> F>
  [[nodiscard]] constexpr auto map(
    F&& functor,
    const std::source_location loc = std::source_location::current()
  ) const& {
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
  template<std::invocable<E> F>
  [[nodiscard]] constexpr auto map_err(
    F&& functor,
    const std::source_location loc = std::source_location::current()
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
  template<std::invocable<T> F>
  [[nodiscard]] constexpr auto and_then(
    F&& functor,
    const std::source_location loc = std::source_location::current()
  ) && {
    using R = std::invoke_result_t<F, T>;

    static_assert(
      crab::result::is_result_type_v<R>,
      "and_then functor parameter must return a Result<T,E> type"
    );

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
  template<std::invocable<T> F>
  [[nodiscard]] constexpr auto and_then(
    F&& functor,
    const std::source_location loc = std::source_location::current()
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
  [[nodiscard]] constexpr auto ok(
    const std::source_location loc = std::source_location::current()
  ) && -> Option<T> {
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
  [[nodiscard]] constexpr auto err(
    const std::source_location loc = std::source_location::current()
  ) && -> Option<E> {
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
  [[nodiscard]] constexpr auto ok(
    const std::source_location loc = std::source_location::current()
  ) const& -> Option<T> {
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
  [[nodiscard]] constexpr auto err(
    const std::source_location loc = std::source_location::current()
  ) const& -> Option<E> {
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

namespace crab {
  namespace result {
    template<error_type Error>
    struct fallible {
      constexpr fallible() = default;

      // Identity
      constexpr auto operator()(auto tuple) const { return tuple; }

      // Pass with Result<T, E>
      template<std::invocable F, std::invocable... Rest>
      constexpr auto operator()(
        auto tuple,
        F&& function,
        Rest&&... other_functions
      ) const requires is_result_type_v<decltype(std::invoke(function))>
      {
        // tuple.take_unchecked();

        using R = decltype(std::invoke(function));
        using FOkType = typename R::OkType;

        static_assert(
          std::same_as<typename R::ErrType, Error>,
          "Cannot have multiple types of errors in fallible chain."
        );

        using ReturnOk = decltype(std::tuple_cat(
          std::move(tuple).unwrap(),
          std::make_tuple(std::invoke(function).unwrap())
        ));

        using Return = decltype(operator()(
          Result<ReturnOk, Error>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::make_tuple(std::invoke(function).unwrap())
          )},
          std::forward<Rest>(other_functions)...
        ));

        if (tuple.is_err()) {
          return Return{std::move(tuple).unwrap_err()};
        }

        Result<FOkType, Error> result = std::invoke(function);

        if (result.is_err()) {
          return Return{std::move(result).unwrap_err()};
        }

        return operator()(
          Result<ReturnOk, Error>{std::tuple_cat(
            std::move(tuple).unwrap(),
            std::make_tuple(std::move(result).unwrap())
          )},
          std::forward<Rest>(other_functions)...
        );
      }

      template<std::invocable F, std::invocable... Rest>
      constexpr auto operator()(
        // Tuple : Result<std:tuple<...>, Error>
        auto tuple,
        F&& function,
        Rest&&... other_functions
      ) const
        requires(ok_type<decltype(std::invoke(function))> and not is_result_type_v<decltype(function())>)
      {
        // tuple.take_unchecked();

        using ReturnOk = decltype(std::tuple_cat(
          std::move(tuple).unwrap(),
          std::make_tuple(std::invoke(function))
        ));

        return std::move(tuple).and_then([&, this](auto tuple) {
          return operator()(
            Result<ReturnOk, Error>{std::tuple_cat(
              std::move(tuple),
              std::make_tuple(std::invoke(function))
            )},
            std::forward<Rest>(other_functions)...
          );
        });
      }
    };

    template<typename>
    struct decay_fallible {};

    template<ok_type T>
    struct decay_fallible<T> {
      using type = T;
    };

    template<ok_type T, error_type E>
    struct decay_fallible<Result<T, E>> {
      using type = T;
    };

    template<std::invocable F>
    using decay_fallible_function =
      typename decay_fallible<std::invoke_result_t<F>>::type;
  } // namespace result

  template<result::error_type E, std::invocable... F>
  constexpr auto fallible( //
    F&&... fallible
  ) -> Result<std::tuple<result::decay_fallible_function<F>...>, E> {
    return result::fallible<E>{}(
      Result<std::tuple<>, E>{
        std::make_tuple(),
      },
      std::forward<F>(fallible)...
    );
  }

  template<result::ok_type T>
  [[nodiscard]] constexpr auto ok(std::type_identity_t<T>&& value) {
    return result::Ok<T>{std::forward<T>(value)};
  }

  template<result::error_type E>
  [[nodiscard]] constexpr auto err(std::type_identity_t<E>&& value) {
    return result::Err<E>{std::forward<E>(value)};
  }

  [[nodiscard]] constexpr auto ok(auto value) {
    return ok<std::remove_cvref_t<decltype(value)>>(std::move(value));
  }

  [[nodiscard]] constexpr auto err(auto value) {
    return err<std::remove_cvref_t<decltype(value)>>(std::move(value));
  }

  template<typename T, typename E>
  [[nodiscard]] constexpr auto unwrap(
    Result<T, E>&& result,
    std::source_location loc = std::source_location::current()
  ) -> T {
    return std::forward<Result<T, E>>(result).unwrap(loc);
  }

  template<typename T, typename E>
  [[nodiscard]] constexpr auto unwrap_err(
    Result<T, E>&& result,
    std::source_location loc = std::source_location::current()
  ) -> E {
    return std::forward<Result<T, E>>(result).unwrap_err(loc);
  }
} // namespace crab

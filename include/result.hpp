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
#include "option.hpp"

namespace crab {
  /**
   * @brief Base error type for use with Result<T, E>
   */
  class Error {
  public:

    Error() = default;

    Error(const Error&) = default;

    Error(Error&&) = default;

    Error& operator=(const Error&) = default;

    Error& operator=(Error&&) = default;

    virtual ~Error() = default;

    /**
     * @brief Stringified error message for logging purposes
     */
    [[nodiscard]] virtual auto what() const -> String = 0;
  };
} // namespace crab

namespace crab::result {
  /**
   * @brief A valid error type.
   */
  template<typename E> concept error_type = std::is_move_constructible_v<E>;

  /**
   * @brief Converts a given error to its stringified representation.
   */
  template<error_type E>
  auto error_to_string(const E& err) -> String {
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
      return String{typeid(E).name()} + "["
           + std::to_string(static_cast<i64>(err)) + "]";
    } else {
      return typeid(E).name();
    }
  }

  /**
   * @brief Type constraint for a type that can be used with Result<T>
   */
  template<typename T> concept ok_type = std::is_move_constructible_v<T>;

  /**
   * @brief Thin wrapper over a value to be given to Result<T,E>(Ok)'s
   * constructor
   */
  template<typename T>
  requires ok_type<T>
  struct Ok {
    using Inner = T;
    T value;

    explicit Ok(T value): value(std::move(value)) {}
  };

  /**
   * @brief Thin wrapper over a value to be given to Result<T,E>(Err)'s
   * constructor
   */
  template<typename E>
  requires error_type<E>
  struct Err {
    using Inner = E;
    E value;

    explicit Err(E value): value(std::move(value)) {}
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

  using ErrType = E;
  using OkType = T;

private:

  struct invalidated {};

  std::variant<Ok, Err, invalidated> inner;

public:

  Result(T from): // NOLINT(*-explicit-constructor)
      Result{Ok{std::move(from)}} {}

  Result(E from): // NOLINT(*-explicit-constructor)
      Result{Err{std::move(from)}} {}

  Result(Ok&& from): // NOLINT(*-explicit-constructor)
      inner{std::forward<Ok>(from)} {}

  Result(Err&& from): // NOLINT(*-explicit-constructor)
      inner{std::forward<Err>(from)} {}

  Result(Result&& from) noexcept: // NOLINT(*-explicit-constructor)
      inner{std::exchange(from.inner, invalidated{})} {}

  Result(const Result& res) requires std::copyable<T> and std::copyable<E>
      : inner{res.inner} {}

  auto operator=(const Result& res) -> Result& //
    requires std::copyable<T> and std::copyable<E>
  {
    if (&res == this) {
      return *this;
    }

    inner = res.inner;

    return *this;
  }

  auto operator=(Result&& res) noexcept -> Result& {
    inner = std::exchange(res.inner, invalidated{});
    return *this;
  }

  auto operator=(Ok&& from) -> Result& {
    inner = std::forward<Ok>(from);
    return *this;
  }

  auto operator=(Err&& from) -> Result& {
    inner = std::forward<Err>(from);
    return *this;
  }

  auto operator=(T&& from) -> Result& {
    return *this = Ok{std::forward<T>(from)}; /* NOLINT(*operator*)*/
  }

  auto operator=(E&& from) -> Result& {
    return *this = Err{std::forward<E>(from)}; /* NOLINT(*operator*)*/
  }

  [[nodiscard]] explicit operator bool() const { return is_ok(); }

  /**
   * @brief Does this result hold an ok value
   */
  [[nodiscard]] auto is_ok() const -> bool {
    return std::holds_alternative<Ok>(inner);
  }

  /**
   * @brief Does this result hold an err value
   */
  [[nodiscard]] auto is_err() const -> bool {
    return std::holds_alternative<Err>(inner);
  }

  /**
   * @brief Move out the Ok value inside, this will panic & crash if there is no
   * contained Ok value
   */
  [[nodiscard]] auto take_unchecked(
    const std::source_location loc = std::source_location::current()
  ) -> T {
    ensure_valid(loc);
    debug_assert_transparent(
      is_ok(),
      std::format(
        "Called take_unchecked on result with Error:\n{}",
        crab::result::error_to_string(get_err_unchecked())
      ),
      loc
    );
    return std::get<Ok>(std::exchange(inner, invalidated{})).value;
  }

  /**
   * @brief Move out the Error value inside, this will panic & crash if there is
   * no contained Error value
   */
  [[nodiscard]] auto take_err_unchecked(
    const std::source_location loc = std::source_location::current()
  ) -> E {
    ensure_valid(loc);
    debug_assert_transparent(
      is_err(),
      "Called take_err_unchecked on result with ok value",
      loc
    );
    return std::get<Err>(std::exchange(inner, invalidated{})).value;
  }

  /**
   * @brief Gets a reference to the contained inner Ok value, if there is no Ok
   * value this will panic and crash.
   */
  [[nodiscard]] auto get_unchecked(
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
  [[nodiscard]] auto get_err_unchecked(
    const std::source_location loc = std::source_location::current()
  ) -> E& {
    ensure_valid(loc);
    debug_assert_transparent(
      is_err(),
      std::format("Called unwrap on Ok value"),
      loc
    );
    return std::get<Err>(inner).value;
  }

  /**
   * @brief Gets a reference to the contained inner Ok value, if there is no Ok
   * value this will panic and crash.
   */
  [[nodiscard]] auto get_unchecked(
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
  [[nodiscard]] auto get_err_unchecked(
    const std::source_location loc = std::source_location::current()
  ) const -> const E& {
    ensure_valid(loc);
    debug_assert_transparent(
      is_err(),
      std::format("Called unwrap on Ok value"),
      loc
    );
    return std::get<Err>(inner).value;
  }

private:

  /**
   * @brief Internal method for preventing use-after-movas
   */
  auto ensure_valid(
    const std::source_location loc = std::source_location::current()
  ) const -> void {
    debug_assert_transparent(
      not std::holds_alternative<invalidated>(inner),
      "Invalid use of moved result",
      loc
    );
  }

public:

  friend auto operator<<(std::ostream& os, const Result& result)
    -> std::ostream& {
    if (result.is_err()) {
      return os << "Err(" << result.get_err_unchecked() << ")";
    }
    return os << "Ok(" << result.get_unchecked() << ")";
  }

  // ===========================================================================
  //                            Monadic Operations
  // ===========================================================================

  /**
   * Consumes self and if not Error, maps the Ok value to a new value
   * @tparam F
   * @return
   */
  template<
    std::invocable<T> F,
    typename R = std::remove_cvref_t<std::invoke_result_t<F, T>>>
  [[nodiscard]] auto map(
    F functor,
    const std::source_location loc = std::source_location::current()
  ) -> Result<R, E> {
    ensure_valid(loc);

    if (is_ok()) {
      return Result<R, E>{functor(take_unchecked(loc))};
    }

    return Result<R, E>{take_err_unchecked(loc)};
  }

  /**
   * Consumes self and if not Ok, maps the Error value to a new value
   * @tparam F
   * @return
   */
  template<
    std::invocable<E> F,
    typename R = std::remove_cvref_t<std::invoke_result_t<F, E>>>
  [[nodiscard]] auto map_err(
    F functor,
    const std::source_location loc = std::source_location::current()
  ) -> Result<T, R> {
    ensure_valid(loc);

    if (is_err()) {
      return Result<T, R>{functor(take_err_unchecked(loc))};
    }

    return Result<T, R>{take_unchecked(loc)};
  }

  /**
   * @brief If result is Ok, run function on the ok value,
   * If the mapped function is Ok(type M), it returns Result<M, Error>
   */
  template<std::invocable<T> F, typename R = std::invoke_result_t<F, T>>
  requires crab::result::is_result_type_v<R>
         and std::same_as<typename R::ErrType, ErrType>
  [[nodiscard]] auto and_then(const F functor
  ) -> Result<typename R::OkType, ErrType> {
    Result<Result<typename R::OkType, ErrType>, ErrType> res =
      this->map(functor);

    if (res.is_err()) {
      return res.take_err_unchecked();
    }

    return res.take_unchecked();
  }

  // Result<i32, E>
  // Result<Result<T, E>, E>
  // result.map(x => x * 2);

  /**
   * @brief Takes Ok value out of this object and returns it, if is Err and not
   * Ok, an empty option will be returned instead.
   *
   * This function is for use when wanting to abstract away any specific error
   * type to simply 'none'.
   */
  auto into_ok() -> Option<OkType> {
    if (is_ok()) {
      return crab::some(take_unchecked());
    }

    return crab::none;
  }

  /**
   * @brief Takes Err value out of this object and returns it, if is Ok and not
   * Err, an empty option will be returned instead.
   *
   * This function is for use when wanting to abstract away any specific Ok type
   * to simply 'none'.
   */
  auto into_err() -> Option<ErrType> {
    if (is_err()) {
      return crab::some(take_err_unchecked());
    }

    return crab::none;
  }
};

namespace crab {
  namespace result {
    template<error_type Error>
    struct fallible {
      constexpr fallible() = default;

      // Identity
      inline auto operator()(auto tuple) const { return tuple; }

      // Pass with Result<T, E>
      template<std::invocable F, std::invocable... Rest>
      inline auto operator()(
        auto tuple,
        const F function,
        const Rest... other_functions
      ) const requires is_result_type_v<decltype(function())>
      {
        // tuple.take_unchecked();

        using R = decltype(function());
        using FOkType = typename R::OkType;

        static_assert(
          std::same_as<typename R::ErrType, Error>,
          "Cannot have multiple types of errors in fallible chain."
        );

        using ReturnOk = decltype(std::tuple_cat(
          tuple.take_unchecked(),
          std::make_tuple(function().take_unchecked())
        ));
        using Return = decltype(operator()(
          Result<ReturnOk, Error>{std::tuple_cat(
            tuple.take_unchecked(),
            std::make_tuple(function().take_unchecked())
          )},
          other_functions...
        ));

        if (tuple.is_err()) {
          return Return{tuple.take_err_unchecked()};
        }

        Result<FOkType, Error> result = function();

        if (result.is_err()) {
          return Return{result.take_err_unchecked()};
        }

        return operator()(
          Result<ReturnOk, Error>{std::tuple_cat(
            tuple.take_unchecked(),
            std::make_tuple(result.take_unchecked())
          )},
          other_functions...
        );
      }

      template<std::invocable F, std::invocable... Rest>
      inline auto operator()(
        // Tuple : Result<std:tuple<...>, Error>
        auto tuple,
        const F function,
        const Rest... other_functions
      ) const
        requires(ok_type<decltype(function())> and not is_result_type_v<decltype(function())>)
      {
        // tuple.take_unchecked();

        using FOkType = decltype(function());

        using ReturnOk = decltype(std::tuple_cat(
          tuple.take_unchecked(),
          std::make_tuple(function())
        ));
        using Return = decltype(operator()(
          Result<ReturnOk, Error>{
            std::tuple_cat(tuple.take_unchecked(), std::make_tuple(function()))
          },
          other_functions...
        ));

        if (tuple.is_err()) {
          return Return{tuple.take_err_unchecked()};
        }

        FOkType result = function();

        return operator()(
          Result<ReturnOk, Error>{std::tuple_cat(
            tuple.take_unchecked(),
            std::make_tuple(std::move(result))
          )},
          other_functions...
        );
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
  auto fallible(const F... fallible
  ) -> Result<std::tuple<result::decay_fallible_function<F>...>, E> {
    return result::fallible<E>{
    }(Result<std::tuple<>, E>{std::make_tuple()}, fallible...);
  }

  template<result::error_type T>
  auto ok(T value) -> result::Ok<T> {
    return result::Ok{std::move(value)};
  }

  template<result::error_type E>
  auto err(E value) -> result::Err<E> {
    return result::Err{std::move(value)};
  }

  template<typename T, typename E>
  auto unwrap(Result<T, E> result) -> T {
    return result.take_unchecked();
  }

  template<typename T, typename E>
  auto unwrap_err(Result<T, E> result) -> E {
    return result.take_err_unchecked();
  }
} // namespace crab

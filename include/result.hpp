//

// Created by bishan_ on 4/23/24.
//

// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <format>
#include <functional>
#include <variant>

#include "crab/debug.hpp"
#include "option.hpp"
#include "result.hpp"

namespace crab {
  class Error {
  public:
    Error() = default;

    Error(const Error &) = default;

    Error(Error &&) = default;

    Error& operator=(const Error &) = default;

    Error& operator=(Error &&) = default;

    virtual ~Error() = default;

    virtual String what() const = 0;
  };
}

namespace crab::result {
  template<typename E>
  concept error_type = std::is_move_constructible_v<E>
                       and (std::is_base_of_v<Error, E>
                            or requires(const E err) {
                              std::cout << err->what() << std::endl;
                            }
                            or requires(const E err) {
                              std::cout << err.what() << std::endl;
                            });

  template<typename E> requires error_type<E>
  auto error_to_string(const E &err) -> String {
    if constexpr (requires { std::cout << err.what() << std::endl; }) {
      return err.what();
    } else if constexpr (requires { std::cout << err->what() << std::endl; }) {
      return err->what();
    } else {
      return "";
    }
  }

  template<typename T>
  concept ok_type = std::is_move_constructible_v<T>;

  template<typename T> requires ok_type<T>
  struct Ok {
    using Inner = T;
    T value;

    explicit Ok(T value) : value(std::move(value)) {}
  };

  template<typename E> requires error_type<E>
  struct Err {
    using Inner = E;
    E value;

    explicit Err(E value) : value(std::move(value)) {}
  };
}

template<typename T, typename E>
class Result final {
  static_assert(crab::result::ok_type<T>, "T is not a valid Ok type, must be move constructible.");
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
  Result(T from) : Result{Ok{std::move(from)}} {}

  Result(E from) : Result{Err{std::move(from)}} {}

  Result(Ok &&from) : inner{std::forward<Ok>(from)} {}

  Result(Err &&from) : inner{std::forward<Err>(from)} {}

  Result(Result &&from) noexcept: inner{std::exchange(from.inner, invalidated{})} {}

  Result(const Result &res) requires std::copyable<T> and std::copyable<E>
    : inner{res.inner} {}

  auto operator=(const Result &res) -> Result& requires std::copyable<T> and std::copyable<E> {
    inner = res.inner;
    return *this;
  }

  auto operator=(Ok &&from) -> Result& {
    inner = std::forward<Ok>(from);
    return *this;
  }

  auto operator=(Err &&from) -> Result& {
    inner = std::forward<Err>(from);
    return *this;
  }

  auto operator=(T &&from) -> Result& {
    return *this = Ok{std::forward<T>(from)};
  }

  auto operator=(E &&from) -> Result& {
    return *this = Err{std::forward<E>(from)};
  }

  [[nodiscard]] operator bool() const { return is_ok(); }

  [[nodiscard]]
  auto is_ok() const -> bool {
    #if DEBUG
    ensure_valid();
    #endif
    return std::holds_alternative<Ok>(inner);
  }

  [[nodiscard]]
  auto is_err() const -> bool {
    #if DEBUG
    ensure_valid();
    #endif
    return std::holds_alternative<Err>(inner);
  }

  [[nodiscard]] auto take_unchecked() -> T {
    T val{std::move(get_unchecked())};
    inner = invalidated{};
    return val;
  }

  [[nodiscard]] auto take_err_unchecked() -> E {
    E val{std::move(get_err_unchecked())};
    inner = invalidated{};
    return val;
  }

  [[nodiscard]] auto get_unchecked() -> T& {
    #if DEBUG
    ensure_valid();
    #endif
    debug_assert(
      is_ok(),
      std::format(
        "Called unwrap on result with Error:\n{}",
        crab::result::error_to_string(get_err_unchecked())
      )
    );
    return std::get<Ok>(inner).value;
  }

  [[nodiscard]] auto get_err_unchecked() -> E& {
    #if DEBUG
    ensure_valid();
    #endif
    debug_assert(is_err(), std::format("Called unwrap on ok value"));

    return std::get<Err>(inner).value;
  }

  [[nodiscard]] auto get_unchecked() const -> const T& {
    #if DEBUG
    ensure_valid();
    #endif
    debug_assert(
      is_ok(),
      std::format("Called unwrap on result with Error:\n{}", crab::result::error_to_string(get_err_unchecked()))
    );
    return std::get<Ok>(inner).value;
  }

  [[nodiscard]] auto get_err_unchecked() const -> const E& {
    #if DEBUG
    ensure_valid();
    #endif
    debug_assert(is_err(), std::format("Called unwrap on ok value"));

    return std::get<Err>(inner).value;
  }

  auto ensure_valid() const -> void {
    debug_assert(!std::holds_alternative<invalidated>(inner), "Invalid use of moved result");
  }

  friend auto operator<<(std::ostream &os, const Result &result) -> std::ostream& {
    #if DEBUG
    result.ensure_valid();
    #endif
    if (result.is_err())
      return os << "Err(" << result.get_err_unchecked() << ")";
    return os << "Ok(" << result.get_unchecked() << ")";
  }

  // Functional / Chaining Methods

  /**
   * Consumes self and if not Error, maps the Ok value to a new value
   * @tparam F
   * @return
   */
  template<std::invocable<T> F, typename R=std::invoke_result_t<F, T>>
  [[nodiscard]] auto map(const F functor) -> Result<R, E> {
    ensure_valid();

    return std::visit(
      crab::cases{
        [functor](Ok ok) -> Result<R, E> { return Result<R, E>{functor(std::move(ok).value)}; },
        [](Err err) -> Result<R, E> { return Result<R, E>{std::move(err).value}; },

        // Unreachable
        [this](invalidated) -> Result<R, E> { return Result<R, E>{take_err_unchecked()}; }
      },
      std::exchange(inner, invalidated{})
    );
  }

  /**
   * Consumes self and if not Ok, maps the Error value to a new value
   * @tparam F
   * @return
   */
  template<std::invocable<E> F, typename R=std::invoke_result_t<F, T>>
  [[nodiscard]] auto map_err(const F functor) -> Result<T, R> {
    ensure_valid();

    return std::visit(
      crab::cases{
        [](Ok ok) -> Result<T, R> { return Result<T, R>{std::move(ok).value}; },
        [functor](Err err) -> Result<T, R> { return Result<T, R>{functor(std::move(err).value)}; },

        // Unreachable
        [this](invalidated) -> Result<T, R> { return Result<T, R>{take_unchecked()}; }
      },
      std::exchange(inner, invalidated{})
    );
  }
};

namespace crab {
  namespace result {
    template<typename>
    struct is_result_type {
      static constexpr bool value = false;
    };

    template<typename T, typename E>
    struct is_result_type<Result<T, E>> {
      static constexpr bool value = true;
    };

    template<typename>
    struct is_ok_type {
      static constexpr bool value = false;
    };

    template<ok_type T>
    struct is_ok_type<Ok<T>> {
      static constexpr bool value = true;
    };

    template<typename>
    struct is_err_type {
      static constexpr bool value = false;
    };

    template<error_type E>
    struct is_err_type<Err<E>> {
      static constexpr bool value = true;
    };

    template<error_type Error>
    struct fallible {
      constexpr fallible() = default;

      // Identity
      auto operator()(auto tuple) const { return tuple; }

      // Pass with Result<T, E>
      template<std::invocable F, std::invocable... Rest>
      auto operator()(
        // Tuple : Result<std:tuple<...>, Error>
        auto tuple,
        const F function,
        const Rest... other_functions
      ) const requires is_result_type<decltype(function())>::value {
        // tuple.take_unchecked();

        using R = decltype(function());
        using FOkType = typename R::OkType;

        static_assert(
          std::same_as<typename R::ErrType, Error>,
          "Cannot have multiple types of errors in fallible chain."
        );

        using ReturnOk = decltype(std::tuple_cat(tuple.take_unchecked(), std::make_tuple(function().take_unchecked())));
        // using Return = std::invoke_result_t<decltype(operator()), Result<ReturnOk, Error>, Rest...>;
        using Return = decltype(operator()(
          Result<ReturnOk, Error>{std::tuple_cat(tuple.take_unchecked(), std::make_tuple(function().take_unchecked()))},
          other_functions...
        ));

        if (tuple.is_err()) return Return{tuple.take_err_unchecked()};

        Result<FOkType, Error> result = function();

        if (result.is_err()) return Return{result.take_err_unchecked()};

        return operator()(
          Result<ReturnOk, Error>{std::tuple_cat(tuple.take_unchecked(), std::make_tuple(result.take_unchecked()))},
          other_functions...
        );
      }

      template<std::invocable F, std::invocable... Rest>
      auto operator()(
        // Tuple : Result<std:tuple<...>, Error>
        auto tuple,
        const F function,
        const Rest... other_functions
      ) const requires (
        ok_type<decltype(function())> and
        not is_result_type<decltype(function())>::value
      ) {
        // tuple.take_unchecked();

        using FOkType = decltype(function());

        using ReturnOk = decltype(std::tuple_cat(tuple.take_unchecked(), std::make_tuple(function())));
        // using Return = std::invoke_result_t<decltype(fallible{}.operator()), Result<ReturnOk, Error>, Rest...>;
        using Return = decltype(operator()(
          Result<ReturnOk, Error>{std::tuple_cat(tuple.take_unchecked(), std::make_tuple(function()))},
          other_functions...
        ));

        if (tuple.is_err()) return Return{tuple.take_err_unchecked()};

        FOkType result = function();

        return operator()(
          Result<ReturnOk, Error>{std::tuple_cat(tuple.take_unchecked(), std::make_tuple(std::move(result)))},
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
    using decay_fallible_function = typename decay_fallible<std::invoke_result_t<F>>::type;
  }

  template<result::error_type E, std::invocable... F>
  auto fallible(
    const F... fallible
    // ) -> Result<std::tuple<typename result::decay_fallible<decltype(fallible())>::type...>, E> {
  ) -> Result<std::tuple<result::decay_fallible_function<F>...>, E> {
    constexpr static result::fallible<E> stateless{};
    return stateless(Result<std::tuple<>, E>{std::make_tuple()}, fallible...);
  }

  template<result::ok_type T, result::error_type E>
  auto to_option(Result<T, E> value) -> Option<T> {
    if (value.is_ok()) {
      return some(value.take_unchecked());
    }

    return none;
  }

  template<typename T>
  auto ok(T value) -> result::Ok<T> {
    return result::Ok{std::move(value)};
  }

  template<typename E>
  auto err(E value) -> result::Err<E> {
    return result::Err{std::move(value)};
  }

  template<typename T, typename E>
  auto unwrap(Result<T, E> result) -> T { return result.take_unchecked(); }

  template<typename T, typename E>
  auto unwrap_err(Result<T, E> result) -> E { return result.take_err_unchecked(); }
}

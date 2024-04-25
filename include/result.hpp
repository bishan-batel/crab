//
// Created by bishan_ on 4/23/24.
//

// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <format>
#include <variant>

#include "debug.hpp"
#include "option.hpp"

namespace crab {
  class Error {
  public:
    Error() = default;

    Error(const Error &) = default;

    Error(Error &&) = default;

    Error &operator=(const Error &) = default;

    Error &operator=(Error &&) = default;

    virtual ~Error() = default;

    virtual StringView what() const = 0;
  };
}

namespace crab::result {
  template<typename E>
  concept is_error_type = std::is_copy_assignable_v<E> and std::is_base_of_v<Error, E>;

  template<typename T>
  concept is_ok_type = std::is_move_constructible_v<T>;

  template<typename T> requires is_ok_type<T>
  struct Ok {
    T value;

    explicit Ok(T value) : value(std::move(value)) {}
  };

  template<typename E> requires is_error_type<E>
  struct Err {
    E value;

    explicit Err(E value) : value(std::move(value)) {}
  };
}

template<typename T, typename E>
  requires crab::result::is_ok_type<E> and crab::result::is_error_type<E>
class Result final {
  std::variant<T, E, unit> inner;

public:
  explicit Result(T from) : inner(from) {}

  explicit Result(E from) : inner(from) {}

  Result(crab::result::Ok<T> &&from) : Result(std::move(from.value)) {}

  Result(crab::result::Err<E> &&from) : Result(std::move(from.value)) {}

  Result(Result &&from) noexcept: inner(std::exchange(from.inner, unit{})) {}

  Result(const Result &) = delete;

  void operator=(const Result &) = delete;

  Result &operator=(crab::result::Ok<T> &&from) {
    inner = from.value;
    return *this;
  }

  Result &operator=(crab::result::Err<E> &&from) {
    inner = from.value;
    return *this;
  }

  [[nodiscard]]

  operator bool() const { return is_ok(); }

  [[nodiscard]]
  __always_inline bool is_ok() const {
    #if DEBUG
    ensure_valid();
    #endif
    return std::holds_alternative<T>(inner);
  }

  [[nodiscard]]
  __always_inline bool is_err() const {
    #if DEBUG
    ensure_valid();
    #endif
    return std::holds_alternative<E>(inner);
  }

  [[nodiscard]] T take_unchecked() {
    T some = std::move(get_unchecked());
    inner = unit{};
    return some;
  }

  [[nodiscard]] E take_err_unchecked() {
    E error = std::move(get_err_unchecked());
    inner = unit{};
    return error;
  }

  [[nodiscard]] T &get_unchecked() {
    #if DEBUG
    ensure_valid();
    #endif
    debug_assert(is_ok(), std::format("Called unwrap on result with Error:\n{}", get_err_unchecked().what()));
    return std::get<T>(inner);
  }

  [[nodiscard]] E &get_err_unchecked() {
    #if DEBUG
    ensure_valid();
    #endif
    debug_assert(is_err(), std::format("Called unwrap on ok value"));

    return std::get<E>(inner);
  }

  [[nodiscard]] const T &get_unchecked() const {
    #if DEBUG
    ensure_valid();
    #endif
    debug_assert(is_ok(), std::format("Called unwrap on result with Error:\n{}", get_err_unchecked().what()));
    return std::get<T>(inner);
  }

  [[nodiscard]] const E &get_err_unchecked() const {
    #if DEBUG
    ensure_valid();
    #endif
    debug_assert(is_err(), std::format("Called unwrap on ok value"));

    return std::get<E>(inner);
  }

  void ensure_valid() const { debug_assert(!std::holds_alternative<unit>(inner), "Invalid use of moved result"); }

  friend std::ostream &operator<<(std::ostream &os, const Result &result) {
    #if DEBUG
    result.ensure_valid();
    #endif
    if (result.is_err())
      return os << result.get_err_unchecked();
    return os << result.get_unchecked();
  }
};

namespace crab {
  template<typename T, typename E>
  Option<T> to_option(Result<T, E> value) {
    if (value.is_ok()) {
      return some(value.take_unchecked());
    }

    return none;
  }

  template<typename T>
  result::Ok<T> ok(T value) {
    return result::Ok{value};
  }

  template<typename E>
  result::Err<E> err(E value) {
    return result::Err{value};
  }

  template<typename T, typename E>
  T unwrap(Result<T, E> result) { return result.take_unchecked(); }

  template<typename T, typename E>
  E unwrap_err(Result<T, E> result) { return result.take_err_unchecked(); }
}

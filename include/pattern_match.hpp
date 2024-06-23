//
// Created by bishan_ on 4/24/24.
//

#pragma once

#include "preamble.hpp"

#include "option.hpp"
#include "result.hpp"

namespace crab {
  namespace pattern {
    template<typename Function, typename... Args>
    concept invocable_with = requires(Function f, Args... p) { f(std::move(p...)); };

    namespace helper {
      template<typename T>
      struct param_type {
        using type = Option<T>;
      };

      template<>
      struct param_type<unit> {
        using type = bool;
      };
    };

    template<typename T=unit>
    class Else {
      typename helper::param_type<T>::type value;

    public:
      explicit Else(typename helper::param_type<T>::type value)
        : value(value) {}

      Else() requires std::is_same_v<Option<T>, decltype(value)>
        : value(none) {}

      Else() requires std::is_same_v<bool, decltype(value)>
        : value(false) {}

      Else(Else &&) = default;

      Else(const Else &) = delete;

      Else& operator=(Else &&) = default;

      Else& operator=(const Else &) = delete;

      ~Else() = default;

      template<typename F> requires std::is_invocable_v<F, T> and std::is_same_v<decltype(value), Option<T>>
      void or_else(F block) {
        if (value.is_none()) return;

        block(value.take_unchecked());
      }

      template<typename F> requires std::is_invocable_v<F> and std::is_same_v<decltype(value), bool>
      void or_else(F block) {
        if (value) {
          block();
        }
      }
    };
  }

  template<typename T, typename F> requires pattern::invocable_with<F, T>
  pattern::Else<> if_some(Option<T> &&option, F block) {
    if (option.is_none()) {
      return pattern::Else{true};
    }

    block(option.take_unchecked());

    return pattern::Else{false};
  }

  template<typename T, typename F> requires std::is_invocable_v<F>
  pattern::Else<T> if_none(Option<T> &&option, F block) {
    if (option.is_some()) {
      return pattern::Else<T>{option.take_unchecked()};
    }

    block();

    return pattern::Else<T>{};
  }

  template<typename T, typename E, typename F> requires pattern::invocable_with<F, T>
  pattern::Else<E> if_ok(Result<T, E> &&result, F block) {
    if (result.is_err()) {
      return pattern::Else<E>{result.take_err_unchecked()};
    }

    block(result.take_unchecked());

    return pattern::Else<E>{};
  }

  template<typename T, typename E, typename F> requires pattern::invocable_with<F, E>
  pattern::Else<T> if_err(Result<T, E> &&result, F block) {
    if (result.is_ok()) {
      return pattern::Else<T>{result.take_unchecked()};
    }

    block(result.take_err_unchecked());

    return pattern::Else<T>{};
  }

  template<typename T, typename F> requires pattern::invocable_with<F, const T&>
  pattern::Else<> if_some(const Option<T> &option, F block) {
    if (option.is_none()) {
      return pattern::Else{true};
    }

    block(option.get_unchecked());

    return pattern::Else{false};
  }

  template<typename T, typename F> requires std::is_invocable_v<F>
  pattern::Else<Ref<T>> if_none(const Option<T> &option, F block) {
    if (option.is_some()) {
      return pattern::Else<Ref<T>>{Ref{option.get_unchecked()}};
    }

    block();

    return pattern::Else<Ref<T>>{};
  }

  template<typename T, typename E, typename F> requires pattern::invocable_with<F, const T&>
  pattern::Else<Ref<E>> if_ok(const Result<T, E> &result, F block) {
    if (result.is_err()) {
      return pattern::Else<Ref<E>>{Ref{result.get_err_unchecked()}};
    }

    block(result.get_unchecked());

    return pattern::Else<Ref<E>>{};
  }

  template<typename T, typename E, typename F> requires pattern::invocable_with<F, const E&>
  pattern::Else<Ref<T>> if_err(const Result<T, E> &result, F block) {
    if (result.is_ok()) {
      return pattern::Else<Ref<T>>{Ref{result.get_unchecked()}};
    }

    block(result.get_err_unchecked());

    return pattern::Else<Ref<T>>{};
  }
}

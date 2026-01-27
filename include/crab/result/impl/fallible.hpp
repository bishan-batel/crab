
#pragma once

#include "crab/collections/Tuple.hpp"
#include "crab/mem/forward.hpp"
#include "crab/mem/move.hpp"
#include "crab/opt/concepts.hpp"
#include "crab/result/concepts.hpp"
#include "crab/ty/functor.hpp"

namespace crab::result::impl {
  template<error_type Error>
  struct fallible {

    template<typename... T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(Tuple<T...> tuple) const {
      return Result<Tuple<T...>, Error>{mem::forward<Tuple<T...>>(tuple)};
    }

    template<typename PrevResults, ty::provider F, ty::provider... Rest>
    requires result_type<ty::functor_result<F>>
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(
      PrevResults tuple /* Tuple<T...>*/,
      F&& function,
      Rest&&... other_functions
    ) const {
      return std::invoke(function).and_then([&]<typename R>(R&& result) {
        return operator()(
          std::tuple_cat(mem::move(tuple), Tuple<R>(mem::forward<R>(result))),
          mem::forward<Rest>(other_functions)...
        );
      });
    }

    template<typename PrevResults, std::invocable F, std::invocable... Rest>
    requires(opt::option_type<std::invoke_result_t<F>> and std::is_default_constructible_v<Error>)
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(
      PrevResults tuple /* Tuple<T...>*/,
      F&& function,
      Rest&&... other_functions
    ) const {
      return std::invoke(function).template ok_or<Error>(
                                    []() -> Error { return Error{}; }
      ).flat_map([&]<typename R>(R&& result) {
        return operator()(
          std::tuple_cat(mem::move(tuple), Tuple<R>(mem::forward<R>(result))),
          mem::forward<Rest>(other_functions)...
        );
      });
    }

    template<typename PrevResults, std::invocable F, std::invocable... Rest>
    requires(not result_type<std::invoke_result_t<F>>)
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(
      PrevResults tuple /* Tuple<T...>*/,
      F&& function,
      Rest&&... other_functions
    ) const {
      return operator()(
        std::tuple_cat(mem::move(tuple), Tuple<std::invoke_result_t<F>>(std::invoke(function))),
        mem::forward<Rest>(other_functions)...
      );
    }

    template<typename PrevResults, typename V, typename... Rest>
    requires(not std::invocable<V> and not result_type<V>)
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(
      PrevResults tuple /* Tuple<T...>*/,
      V&& value,
      Rest&&... other_functions
    ) const {
      return operator()(
        std::tuple_cat(mem::move(tuple), Tuple<V>{mem::forward<V>(value)}),
        mem::forward<Rest>(other_functions)...
      );
    }

    template<typename PrevResults, typename V, typename... Rest>
    requires(not std::invocable<V>)
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(
      PrevResults tuple, /* Tuple<T...>*/
      Result<V, Error> value,
      Rest&&... other_functions
    ) const {
      return mem::move(value).and_then([&]<typename R>(R&& result) {
        return operator()(
          std::tuple_cat(mem::move(tuple), Tuple<R>(mem::forward<R>(result))),
          mem::forward<Rest>(other_functions)...
        );
      });
    }
  };
}

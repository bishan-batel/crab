/// @file crab/opt/impl/fallible.hpp
/// @ingroup opt
/// @internal
#pragma once

#include "crab/opt/Option.hpp"
#include "crab/mem/forward.hpp"

namespace crab::opt::impl {
  /// implementation for crab::opt::fallible
  ///
  /// TODO: rework this, this is the same implementation as a naive Kishan 2 years ago.
  ///
  /// @ingroup opt
  /// @internal
  struct fallible final {
    template<typename... T>
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(Tuple<T...> tuple) const {
      return Option<Tuple<T...>>{mem::forward<Tuple<T...>>(tuple)};
    }

    template<typename PrevResults, ty::provider F, typename... Rest>
    requires option_type<ty::functor_result<F>>
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(
      PrevResults tuple /* Tuple<T...>*/,
      F&& function,
      Rest&&... other_functions
    ) const {
      return std::invoke(function).flat_map([&]<typename R>(R&& result) {
        return operator()(
          std::tuple_cat(mem::move(tuple), Tuple<R>(mem::forward<R>(result))),
          mem::forward<Rest>(other_functions)...
        );
      });
    }

    template<typename PrevResults, ty::provider F, typename... Rest>
    requires(not option_type<ty::functor_result<F>>)
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(
      PrevResults tuple /* Tuple<T...>*/,
      F&& function,
      Rest&&... other_functions
    ) const {
      return operator()(
        std::tuple_cat(mem::move(tuple), Tuple<ty::functor_result<F>>(std::invoke(function))),
        mem::forward<Rest>(other_functions)...
      );
    }

    template<typename PrevResults, typename V, typename... Rest>
    requires(not ty::provider<V> and not option_type<V>)
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
    requires(not ty::provider<V>)
    [[nodiscard]] CRAB_INLINE constexpr auto operator()(
      PrevResults tuple, /* Tuple<T...>*/
      Option<V> value,
      Rest&&... other_functions
    ) const {
      return mem::move(value).flat_map([&](V&& result) {
        return operator()(
          std::tuple_cat(mem::move(tuple), std::tuple<V>(mem::forward<V>(result))),
          mem::forward<Rest>(other_functions)...
        );
      });
    }
  };
}

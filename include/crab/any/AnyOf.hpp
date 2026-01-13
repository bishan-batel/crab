#pragma once

#include <utility>
#include "crab/any/impl/AnyOfStorage.hpp"
#include "crab/type_traits.hpp"

namespace crab::any {

  namespace impl {
  }

  template<typename... Ts>
  class AnyOf final {
    static_assert((ty::movable<Ts> and ...), "Cannot construct an AnyOf with an immovable Type");
    static_assert(sizeof...(Ts) > 0, "Cannot create an AnyOf with novariants");

  public:

    template<usize I>
    using NthType = ty::nth_type<I, Ts...>;

    template<typename T>
    requires ty::either<T, Ts...>
    consteval auto index_of_type() -> usize {

      return a;
    };

  private:

    impl::AnyOfStorage<Ts...> storage;
  };
}

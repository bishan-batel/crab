#pragma once

#include <type_traits>
#include "crab/type_traits.hpp"

namespace crab::any::impl {

  template<typename Visitor, typename... Ts>
  concept VisitorForTypes = (ty::consumer<Visitor, Ts> and ...);

  template<typename Visitor, typename... Ts>
  requires VisitorForTypes<Visitor, Ts...>
  using VisitorResultType = std::common_type_t<ty::functor_result<Visitor, Ts>...>;
}

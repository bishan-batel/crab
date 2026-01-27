#pragma once

#include <type_traits>
#include "crab/ty/functor.hpp"

namespace crab::any::impl {

  template<typename Visitor, typename... Ts>
  concept VisitorForTypes = (ty::consumer<Visitor, Ts> and ...);

  template<typename Visitor, typename... Ts>
  requires VisitorForTypes<Visitor, Ts...>
  using VisitorResultType = std::common_type_t<ty::functor_result<Visitor, Ts>...>;
}

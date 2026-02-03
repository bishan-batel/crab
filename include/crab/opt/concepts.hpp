/// @file crab/opt/concepts.hpp
/// @ingroup opt

#pragma once

#include "crab/opt/forward.hpp"
#include "crab/ty/bool_types.hpp"

namespace crab::opt {
  namespace impl {
    /// Type predicate helper for if the given type T is an option type, false
    /// for most types
    /// @ingroup opt
    /// @internal
    template<typename>
    struct is_option_type final : ty::false_type {};

    /// Type predicate helper for if the given type is an option type, true
    /// only if the given type is of the form Option<T> for some T
    /// @ingroup opt
    /// @internal
    template<typename T>
    struct is_option_type<opt::Option<T>> final : ty::true_type {};

  }

  /// Type predicate for if the given type T is some form of crab Option
  /// @ingroup opt
  /// @hideinitializer
  template<typename T>
  concept option_type = impl::is_option_type<T>::value;

}

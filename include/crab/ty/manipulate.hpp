/// @file manipulate.hpp
/// @ingroup ty
/// Basic type manipulation for things like removing reference qualifiers.
#pragma once

#include <type_traits>

/// @addtogroup
/// @{

namespace crab::ty {
  /// Metafunction for turning const T& or T& -> T, or leave T alone if T
  /// is not a reference.
  template<typename T>
  using remove_reference = std::remove_reference_t<T>;

}

/// }@

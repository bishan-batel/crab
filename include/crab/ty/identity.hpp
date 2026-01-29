/// @file identity.hpp
/// @ingroup ty
///
/// Identity meta-template function

#pragma once

/// @addtogroup ty
/// @{

namespace crab::ty {

  namespace impl {

    /// @internal
    /// Helper for crab::ty::identity
    template<typename T>
    struct identity final {
      using type = T;
    };
  }

  /// Identity metafunction, is equal to the input.
  ///
  /// This can be used for special cases when metatemplate programming, or when
  /// you are making a templated function and you want to
  template<typename T>
  using identity = typename impl::identity<T>::type;
}

/// }@

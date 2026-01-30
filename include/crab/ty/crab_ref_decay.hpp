/// @file crab/ty/crab_ref_decay.hpp
/// @ingroup ty
/// Crab-meta metatemplate constraints relating to crab::ref::Ref and crab::ref::RefMut.

#pragma once

#include "crab/core.hpp"
#include "crab/ref/forward.hpp"
#include "crab/ty/bool_types.hpp"

/// @addtogroup ty
/// @{

namespace crab::ty {
  namespace impl {

    /// @internal
    /// Type predicate for if T is an immutable crab reference type
    template<typename T>
    struct is_crab_ref final : false_type {};

    /// @internal
    /// T of the form Ref<T> is a immutable crab reference type
    template<typename T>
    struct is_crab_ref<::crab::ref::Ref<T>> final : true_type {};

    /// @internal
    /// Type predicate for if T is an mutable crab reference type
    template<typename T>
    struct is_crab_ref_mut final : false_type {};

    /// @internal
    /// T of the form RefMut<T> is a mutable crab reference type
    template<typename T>
    struct is_crab_ref_mut<::crab::ref::RefMut<T>> final : true_type {};

    /// @internal
    /// helper type constructor for crab::ref_decay
    template<typename T>
    struct crab_ref_decay final {
      using type = T;
    };

    /// @internal
    /// Specialisation for crab::ref_decay for Ref<T> -> const T&
    template<typename T>
    struct crab_ref_decay<::crab::ref::Ref<T>> final {
      using type = const T&;
    };

    /// @internal
    /// Specialisation for crab::ref_decay for RefMut<T> -> T&
    template<typename T>
    struct crab_ref_decay<::crab::ref::RefMut<T>> final {
      using type = T&;
    };

  }

  /// Type function that will turn the given type of for Ref<T>/RefMut<T>
  /// into const T& / T&. If the given type is not a crab reference wrapper, then
  /// this will simply be aliased to T
  template<typename T>
  using crab_ref_decay = impl::crab_ref_decay<T>::type;

  /// Type predicate for whether or not the given type T is a crab
  /// immutable reference, eg. of the form crab::ref::Ref<T>
  template<typename T>
  concept crab_ref = impl::is_crab_ref<T>::value;

  /// Type predicate for whether or not the given type T is a crab
  /// mutable reference, eg. of the form crab::ref::RefMut<T>
  template<typename T>
  concept crab_ref_mut = impl::is_crab_ref_mut<T>::value;

  /// Type predicate for whether or not the given type T is any form of
  /// crab reference wrapper.
  template<typename T>
  concept any_crab_ref = crab_ref<T> or crab_ref_mut<T>;
}

/// }@

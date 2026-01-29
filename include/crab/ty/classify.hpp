/// @file classify.hpp
/// crab::ty metatemplate types for general classification of types (not_void, complete type, cvref qualifications).

#pragma once

#include <type_traits>
#include "crab/ty/bool_types.hpp"
#include "crab/ty/compare.hpp"

namespace crab::ty {

  /// True if the given type is a complete type.
  /// ex. not a forward declaration and can be used fully
  template<typename T>
  concept complete_type = requires { sizeof(T); };

  /// Requirement for a type to not be 'void'
  template<typename T>
  concept not_void = different_than<T, void>;

  /// This is true for a given type T if that type is not qualified
  /// with 'const' or 'volatile.
  template<typename T>
  concept unqualified = different_than<T, std::remove_cv_t<T>>;

  namespace impl {
    /// @internal
    /// Helper for crab::ty::is_const
    template<typename T>
    struct is_const : false_type {};

    /// @internal
    /// Helper for crab::ty::is_const
    template<typename T>
    struct is_const<const T> : true_type {};
  }

  /// Requirement for the type to be 'const' (const T, const T&)
  template<typename T>
  concept is_const = impl::is_const<T>::value;

  /// Requirement for the type to not be 'const' (T, T&).
  template<typename T>
  concept non_const = not is_const<T>;

  namespace impl {
    /// @internal
    /// Helper for crab::ty::is_volatile
    template<typename T>
    struct is_volatile : false_type {};

    /// @internal
    /// Helper for crab::ty::is_volatile
    template<typename T>
    struct is_volatile<volatile T> : true_type {};
  }

  /// Requirement for the type to be volatile (volatile T, volatile T&, ...)
  template<typename T>
  concept is_volatile = impl::is_volatile<T>::value;

  /// Requirement for the type to be volatile (T,  T&, ...).
  template<typename T>
  concept non_volatile = not is_volatile<T>;

  namespace impl {
    /// @internal
    /// Helper for crab::ty::is_reference
    template<typename T>
    struct is_reference : false_type {};

    /// @internal
    /// Helper for crab::ty::is_reference
    template<typename T>
    struct is_reference<T&> : true_type {};
  }

  /// Requirement for the type to be some reference type (T&, const T&).
  /// Note that this is C++ references, this will not ring true for crab
  /// reference types Ref<T> & RefMut<T>
  template<typename T>
  concept is_reference = std::is_reference_v<T>;

  /// Requirement for the given type to not be a reference type
  template<typename T>
  concept non_reference = not is_reference<T>;

  /// Requirement that the given type is a native array type
  template<typename T>
  concept array = std::is_array_v<T>;
}

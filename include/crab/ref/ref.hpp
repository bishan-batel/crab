/// @file crab/ref/ref.hpp
/// @ingroup ref

#pragma once

#include <functional>

#include "crab/assertion/check.hpp"
#include "crab/ty/classify.hpp"
#include "crab/hash/hash.hpp"
#include "fmt/base.h"

// ReSharper disable CppNonExplicitConversionOperator
// ReSharper disable CppNonExplicitConvertingConstructor
// NOLINTBEGIN(*explicit*)

/// @defgroup ref Reference Utilities
/// This group pertains to crab's reference wrappers (Ref and RefMut), as well as general reference utilities, such
/// as construction from a pointer into a raw value, implicit_cast, dynamic casts, etc.

/// @namespace crab::ref
/// @ingroup ref
/// This namespace contains reference utilities and alike, see the @ref ref group for more information.
namespace crab::ref {

  /// Immutable Reference to some type T that is always non-null, use in place of 'const T&'
  /// when applicable (for ex. inside template parameters of STL types).
  ///
  /// This type is (by default) exposed in crab's prelude, so you do not need to fully qualify its name.
  ///
  /// Note that if you wish to have a *mutable* reference, see @ref RefMut
  ///
  /// @invariant A Ref will *never* be null.
  /// @ingroup ref
  /// @ingroup prelude
  template<typename T>
  class Ref final {
    static_assert(ty::non_const<T>, "Cannot have a Ref<T> of a const T, Ref<T> always represents a constant reference");

    static_assert(ty::non_reference<T>, "Cannot have a Ref<T> to another reference type T");

    /// Internal construction from a raw pointer
    /// @internal
    CRAB_INLINE CRAB_NONNULL constexpr explicit Ref(
      const T* const pointer,
      const SourceLocation loc = SourceLocation::current()
    ):
        pointer(pointer) {
      crab_dbg_check_with_location(pointer, loc, "Invalid State: Cannot create a NULL Ref object");
    }

  public:

    /// Constructs a reference from the given pointer
    ///
    /// # Panics
    /// This will panic if the given pointer is null.
    [[nodiscard]] CRAB_INLINE CRAB_NONNULL constexpr static Ref from_raw(const T* const pointer) {
      return Ref{pointer};
    }

    /// Construction from a const ref
    ///
    /// # Panics
    /// This will panic if the given reference is nullptr, note that this panic may be optimised out by compilers as it
    /// is *clear undefined behavior* if the reference given is null.
    ///
    /// This means that if you try to construct a ref from nullptr
    CRAB_INLINE constexpr Ref(const T& ref): Ref{&ref} {}

    /// Deleted construction, you *cannot* construct a reference to a xalue
    Ref(T&& ref) = delete;

    /// Deleted construction, you *cannot* construct a reference to a rvalue
    Ref(const T&& ref) = delete;

    /// Implicit conversion from Ref<T> into a const T&
    [[nodiscard]] CRAB_INLINE constexpr operator const T&() const {
      return get_ref();
    };

    /// Iplicit conversion from Ref<T> into a const T*
    [[nodiscard]] CRAB_INLINE constexpr operator const T*() const {
      return as_ptr();
    };

    /// Derefernces a Ref will lead into a C++ reference to the inner value
    [[nodiscard]] CRAB_INLINE constexpr const T& operator*() const {
      return get_ref();
    }

    /// Arrow operator overload for access to the inner value this is referencing.
    [[nodiscard]] CRAB_INLINE constexpr const T* operator->() const {
      return as_ptr();
    }

    /// Spaceship operator between two refs is the same as the operator between the underlying *pointers*
    [[nodiscard]] CRAB_INLINE constexpr auto operator<=>(const Ref& other) -> decltype(auto) {
      return as_ptr() <=> other.as_ptr();
    }

    /// Gets underlying pointer, this pointer is always non null
    [[nodiscard]] CRAB_INLINE constexpr const T* as_ptr() const {
      return pointer;
    }

    /// Gets a C++ reference to underlying data
    [[nodiscard]] CRAB_INLINE constexpr const T& get_ref() const {
      return *pointer;
    }

    /// Pipe operator to a ostream
    friend CRAB_INLINE constexpr auto operator<<(std::ostream& os, const Ref& val) -> std::ostream& {
      if constexpr (requires(const T& val) { os << val; }) {
        return os << *val;
      } else {
        return os << val.as_ptr();
      }
    }

  private:

    /// Underlying pointer
    /// @internal
    const T* pointer;
  };

  /// Mutable Reference to some type T that is always non-null, use in place of 'T&'
  /// when applicable (for ex. inside template parameters of STL types).
  ///
  /// This type is (by default) exposed in crab's prelude, so you do not need to fully qualify its name.
  ///
  /// Note that if you wish to have a *immutable* reference, see @ref Ref.
  /// However, it is recomended to prefer immutability when possible.
  ///
  /// @invariant A RefMut will *never* be null.
  /// @ingroup ref
  template<typename T>
  class RefMut final {
    static_assert(ty::non_const<T>, "Cannot have a Ref<T> of a const T, Ref<T> always represents a constant reference");

    static_assert(ty::non_reference<T>, "Cannot have a Ref<T> to another reference type T");

    /// Internal construction from a raw pointer
    /// @internal
    CRAB_INLINE CRAB_NONNULL constexpr explicit RefMut(
      T* const pointer,
      const SourceLocation loc = SourceLocation::current()
    ):
        pointer(pointer) {
      crab_dbg_check_with_location(pointer, loc, "Invalid State: Cannot create a NULL RefMut object");
    }

  public:

    /// Constructs a reference from the given pointer
    ///
    /// # Panics
    /// This will panic if the given pointer is null.
    [[nodiscard]] CRAB_INLINE CRAB_NONNULL constexpr static RefMut from_raw(T* const pointer) {
      return RefMut{pointer};
    }

    /// Construction from a const ref
    ///
    /// # Panics
    /// This will panic if the given reference is nullptr, note that this panic may be optimised out by compilers as it
    /// is *clear undefined behavior* if the reference given is null.
    ///
    /// This means that if you try to construct a ref from nullptr
    CRAB_INLINE constexpr RefMut(T& ref): RefMut(&ref) {}

    /// Implicit conversion from RefMut<T> into a const T&
    [[nodiscard]] CRAB_INLINE constexpr operator T&() const {
      return get_mut_ref();
    };

    /// Iplicit conversion from RefMut<T> into a const T*
    [[nodiscard]] CRAB_INLINE constexpr operator T*() const {
      return as_ptr();
    };

    /// Implicit conversion from RefMut<T> -> Ref<T> as it is always possible to add 'const'-ness, however it is not
    /// legal to go the other way aroud (other than through const_cast, which is not recommended).
    [[nodiscard]] CRAB_INLINE constexpr operator Ref<T>() const {
      return as_ref();
    };

    /// Derefernces a RefMut will lead into a C++ reference to the inner value
    [[nodiscard]] CRAB_INLINE constexpr T& operator*() const {
      return get_mut_ref();
    }

    /// Arrow operator overload for access to the inner value this is referencing.
    [[nodiscard]] CRAB_INLINE constexpr T* operator->() const {
      return as_ptr();
    }

    /// Spaceship operator between two refs is the same as the operator between the underlying *pointers*
    [[nodiscard]] CRAB_INLINE constexpr auto operator<=>(const RefMut& other) -> decltype(auto) {
      return as_ptr() <=> other.as_ptr();
    }

    /// Gets underlying pointer, this pointer is always non null
    [[nodiscard]] CRAB_RETURNS_NONNULL CRAB_INLINE constexpr T* as_ptr() const {
      return pointer;
    }

    /// Gets a C++ mutable reference to underlying data
    [[nodiscard]] CRAB_INLINE constexpr T& get_mut_ref() const {
      return *pointer;
    }

    /// Gets a C++ reference to underlying data
    [[nodiscard]] CRAB_INLINE constexpr const T& get_ref() const {
      return *pointer;
    }

    /// Converts RefMut into a immutable reference
    [[nodiscard]] CRAB_INLINE constexpr Ref<T> as_ref() const {
      return Ref<T>(get_mut_ref());
    }

    /// Pipe operator to a ostream
    friend CRAB_INLINE constexpr auto operator<<(std::ostream& os, const RefMut& val) -> std::ostream& {
      if constexpr (requires(const T& val) { os << val; }) {
        return os << *val;
      } else {
        return os << val.as_ptr();
      }
    }

  private:

    /// Underlying pointer
    /// @internal
    T* pointer;
  };

  /// Specialisation for Ref<T> to be formattable if T is formattable
  template<fmt::formattable T>
  [[nodiscard]] auto format_as(Ref<T> ref) -> const T& {
    return *ref;
  }

  template<fmt::formattable T>
  [[nodiscard]] auto format_as(RefMut<T> ref) -> T& {
    return *ref;
  }
}

/// Hasher implementation for Ref<T> is identical for std::hash<T*>
/// @ingroup ref
template<typename T>
struct std::hash<::crab::ref::Ref<T>> {
  /// convert a Ref<T> into a hash code
  /// @internal
  [[nodiscard]] CRAB_INLINE constexpr auto operator()(const ::crab::ref::Ref<T>& var) const -> usize {
    return crab::hash(var.as_ptr());
  };
};

/// Hasher implementation for RefMut<T> is identical for std::hash<T*>
/// @ingroup ref
template<typename T>
struct std::hash<::crab::ref::RefMut<T>> {
  /// convert a Ref<T> into a hash code
  /// @internal
  [[nodiscard]] CRAB_INLINE constexpr auto operator()(const ::crab::ref::RefMut<T>& var) const -> usize {
    return crab::hash(var.as_ptr());
  };
};

namespace crab::prelude {
  using crab::ref::Ref;
  using crab::ref::RefMut;
}

CRAB_PRELUDE_GUARD;

// NOLINTEND(*explicit*)

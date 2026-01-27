#pragma once

#include <functional>

#include "crab/assertion/assert.hpp"
#include "crab/ty/classify.hpp"

// NOLINTBEGIN(*explicit*)

namespace crab::ref {

  /**
   * Reference to some type T that is always NON NULL, use in place of 'const T&'
   * when applicable (for ex. inside template parameters)
   */
  template<typename T>
  class Ref final {
    static_assert(ty::non_const<T>, "Cannot have a Ref<T> of a const T, Ref<T> always represents a constant reference");

    static_assert(ty::non_reference<T>, "Cannot have a Ref<T> to another reference type T");

    CRAB_INLINE constexpr explicit Ref(const T* const pointer, SourceLocation loc = SourceLocation::current()):
        pointer(pointer) {
      debug_assert_transparent(pointer, loc, "Invalid State: Cannot create a NULL Ref object");
    }

  public:

    [[nodiscard]] CRAB_INLINE constexpr static Ref from_unchecked(const T* const pointer) {
      return Ref(pointer);
    }

    CRAB_INLINE constexpr Ref(const T& ref): Ref(&ref) {}

    /**
     * You cannot construct a reference to a xalue
     */
    Ref(T&& ref) = delete;

    /**
     * You cannot construct a reference to an rvalue
     */
    Ref(const T&& ref) = delete;

    [[nodiscard]] CRAB_INLINE constexpr operator const T&() const {
      return get_ref();
    };

    [[nodiscard]] CRAB_INLINE constexpr operator const T*() const {
      return as_ptr();
    };

    [[nodiscard]] CRAB_INLINE constexpr const T& operator*() const {
      return get_ref();
    }

    [[nodiscard]] CRAB_INLINE constexpr const T* operator->() const {
      return as_ptr();
    }

    /**
     * Spaceship operator between two refs is the same as the operator between the underlying *pointers*
     */
    [[nodiscard]] CRAB_INLINE constexpr auto operator<=>(const Ref& other) -> decltype(auto) {
      return as_ptr() <=> other.as_ptr();
    }

    /**
     * Gets underlying pointer, this pointer is always non null
     */
    [[nodiscard]] CRAB_INLINE constexpr const T* as_ptr() const {
      return pointer;
    }

    /**
     * Gets a C++ reference to underlying data
     */
    [[nodiscard]] CRAB_INLINE constexpr const T& get_ref() const {
      return *pointer;
    }

    friend CRAB_INLINE constexpr auto operator<<(std::ostream& os, const Ref& val) -> std::ostream& {
      if constexpr (requires(const T& val) { os << val; }) {
        return os << *val;
      } else {
        return os << val.as_ptr();
      }
    }

  private:

    const T* pointer;
  };

  template<typename T>
  class RefMut final {
    static_assert(ty::non_const<T>, "Cannot have a Ref<T> of a const T, Ref<T> always represents a constant reference");

    static_assert(ty::non_reference<T>, "Cannot have a Ref<T> to another reference type T");

    CRAB_INLINE constexpr explicit RefMut(T* const pointer, SourceLocation loc = SourceLocation::current()):
        pointer(pointer) {
      debug_assert_transparent(pointer, loc, "Invalid State: Cannot create a NULL RefMut object");
    }

  public:

    CRAB_INLINE constexpr RefMut(T& ref): RefMut(&ref) {}

    [[nodiscard]] CRAB_INLINE constexpr static RefMut from_unchecked(T* const pointer) {
      return RefMut(pointer);
    }

    [[nodiscard]] CRAB_INLINE constexpr operator T&() const {
      return get_mut_ref();
    };

    [[nodiscard]] CRAB_INLINE constexpr operator T*() const {
      return as_ptr();
    };

    [[nodiscard]] CRAB_INLINE constexpr operator Ref<T>() const {
      return as_ref();
    };

    [[nodiscard]] CRAB_INLINE constexpr T& operator*() const {
      return get_mut_ref();
    }

    [[nodiscard]] CRAB_INLINE constexpr T* operator->() const {
      return as_ptr();
    }

    /**
     * Spaceship operator between two refs is the same as the operator between the underlying *pointers*
     */
    [[nodiscard]] CRAB_INLINE constexpr auto operator<=>(const RefMut& other) -> decltype(auto) {
      return as_ptr() <=> other.as_ptr();
    }

    /**
     * Gets underlying pointer, this pointer is always non null
     *
     */
    [[nodiscard]] CRAB_RETURNS_NONNULL CRAB_INLINE constexpr T* as_ptr() const {
      return pointer;
    }

    /**
     * Gets a C++ reference to underlying data
     */
    [[nodiscard]] CRAB_INLINE constexpr T& get_mut_ref() const {
      return *pointer;
    }

    /**
     * Gets a C++ reference to underlying data
     */
    [[nodiscard]] CRAB_INLINE constexpr const T& get_ref() const {
      return *pointer;
    }

    /**
     * Converts RefMut into a immutable reference
     */
    [[nodiscard]] CRAB_INLINE constexpr Ref<T> as_ref() const {
      return Ref<T>(get_mut_ref());
    }

    friend CRAB_INLINE constexpr auto operator<<(std::ostream& os, const RefMut& val) -> std::ostream& {
      if constexpr (requires(const T& val) { os << val; }) {
        return os << *val;
      } else {
        return os << val.as_ptr();
      }
    }

  private:

    T* pointer;
  };
}

/**
 * @brief Hash Implementation for Ref<T> is identical for std::hash<T>
 */
template<typename T>
struct std::hash<::crab::ref::Ref<T>> {
  [[nodiscard]] CRAB_INLINE constexpr auto operator()(const ::crab::ref::Ref<T>& mut) const -> usize {
    return std::hash<const T*>{}(mut.as_ptr());
  };
};

template<typename T>
struct std::hash<::crab::ref::RefMut<T>> {
  [[nodiscard]] CRAB_INLINE constexpr auto operator()(const ::crab::ref::RefMut<T>& mut) const -> usize {
    return std::hash<const T*>{}(mut.as_ptr());
  };
};

namespace crab::prelude {
  using crab::ref::Ref;
  using crab::ref::RefMut;
}

CRAB_PRELUDE_GUARD;

// NOLINTEND(*explicit*)

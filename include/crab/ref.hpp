#pragma once

#include <functional>

#include <crab/debug.hpp>
#include <crab/type_traits.hpp>
#include <source_location>

// NOLINTBEGIN(*explicit*)

/**
 * Reference to some type T that is always NON NULL, use in place of 'const T&'
 * when applicable (for ex. inside template parameters)
 */
template<crab::ref::is_valid_type T>
class Ref final {
  constexpr explicit Ref(
    const T* const pointer,
    SourceLocation loc = SourceLocation::current()
  ):
      pointer(pointer) {
    debug_assert_transparent(
      pointer,
      loc,
      "Invalid State: Cannot create a NULL Ref object"
    );
  }

public:

  [[nodiscard]] constexpr static Ref from_unchecked(const T* const pointer) {
    return Ref(pointer);
  }

  constexpr Ref(const T& ref): Ref(&ref) {}

  [[nodiscard]] constexpr operator const T&() const { return get_ref(); };

  [[nodiscard]] constexpr operator const T*() const { return as_ptr(); };

  [[nodiscard]] constexpr const T& operator*() const { return get_ref(); }

  [[nodiscard]] constexpr const T* operator->() const { return as_ptr(); }

  /**
   * Gets underlying pointer, this pointer is always non null
   */
  [[nodiscard]] constexpr const T* as_ptr() const { return pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr const T& get_ref() const { return *pointer; }

  friend constexpr auto operator<<(std::ostream& os, const Ref& val)
    -> std::ostream& {
    if constexpr (requires(const T& val) { os << val; }) {
      return os << *val;
    } else {
      return os << val.as_ptr();
    }
  }

private:

  const T* pointer;
};

template<crab::ref::is_valid_type T>
class RefMut final {
  constexpr explicit RefMut(
    T* const pointer,
    SourceLocation loc = SourceLocation::current()
  ):
      pointer(pointer) {
    debug_assert_transparent(
      pointer,
      loc,
      "Invalid State: Cannot create a NULL RefMut object"
    );
  }

public:

  constexpr RefMut(T& ref): RefMut(&ref) {}

  [[nodiscard]] constexpr static RefMut from_unchecked(T* const pointer) {
    return RefMut(pointer);
  }

  [[nodiscard]] constexpr operator T&() const { return get_mut_ref(); };

  [[nodiscard]] constexpr operator T*() const { return as_ptr(); };

  [[nodiscard]] constexpr operator Ref<T>() const { return as_ref(); };

  [[nodiscard]] constexpr T& operator*() const { return get_mut_ref(); }

  [[nodiscard]] constexpr T* operator->() const { return as_ptr(); }

  /**
   * Gets underlying pointer, this pointer is always non null
   *
   */
  [[nodiscard]] constexpr T* as_ptr() const { return pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr T& get_mut_ref() const { return *pointer; }

  /**
   * Gets a C++ reference to underlying data
   */
  [[nodiscard]] constexpr const T& get_ref() const { return *pointer; }

  /**
   * Converts RefMut into a immutable reference
   */
  [[nodiscard]] constexpr Ref<T> as_ref() const {
    return Ref<T>(get_mut_ref());
  }

  friend constexpr auto operator<<(std::ostream& os, const RefMut& val)
    -> std::ostream& {
    if constexpr (requires(const T& val) { os << val; }) {
      return os << *val;
    } else {
      return os << val.as_ptr();
    }
  }

private:

  T* pointer;
};

/**
 * @brief Hash Implementation for Ref<T> is identical for std::hash<T>
 */
template<typename T>
struct std::hash<Ref<T>> {
  [[nodiscard]] constexpr auto operator()(const Ref<T>& mut) const -> usize {
    return std::hash<const T*>(mut.as_ptr());
  };
};

template<typename T>
struct std::hash<RefMut<T>> : std::hash<Ref<T>> {};

// NOLINTEND(*explicit*)

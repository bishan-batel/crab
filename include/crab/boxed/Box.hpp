/// @file crab/boxed/Box.hpp
/// @ingroup boxed

// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <concepts>
#include <iostream>

#include <utility>

#include "crab/core/unsafe.hpp"
#include "crab/assertion/check.hpp"
#include "crab/core.hpp"
#include "crab/mem/take.hpp"
#include "crab/ref/implicit_cast.hpp"
#include "crab/ref/ref.hpp"
#include "crab/ref/casts.hpp"
#include "crab/ref/from_ptr.hpp"

#include "crab/opt/forward.hpp"

#include "crab/boxed/forward.hpp"
#include "crab/boxed/impl/BoxStorage.hpp"

// NOLINTBEGIN(*explicit*)

namespace crab {
  /// @addtogroup boxed
  /// @{

  /// Storage type specialization for Box<T>
  template<typename T>
  struct opt::Storage<::crab::boxed::Box<T>> final {
    using type = boxed::impl::BoxStorage<T>;
  };

  namespace boxed {
    /// Owned Pointer (RAII) to an instance of T on the heap. This is an owned smart pointer type with no interior
    /// mutability and and who is always non-null for a given valid value of Box<T>.
    ///
    /// # Differences from `std::unique_ptr`
    ///
    /// A moved-from box does not leave it in a "valid but unspecified state" like other "well-behaved" types in the STL
    /// (circa 2024), but leaves it in a state where it is partially-formed (only safe to reassign or destroy).
    ///
    /// If you are using this as a field of a class and don't want your class to be in this 'only safe to reassign or
    /// destroy' state once moved, you must take on the weaker invariant and use Option<Box<T>> to explicitly allowed
    /// Box<T> to be none.
    ///
    /// This is a replacement for std::unique_ptr, with two key differences:
    ///
    /// - Prevents Interior Mutability, being passed a const Box<T>& means dealing
    /// with a const T&
    ///
    /// - A Box<T> has the invariant of always being non null, if it is then you
    /// messed up with move semantics.
    ///
    /// # Examples
    /// ```cpp
    ///  void some_function(Box<u32>);
    ///
    ///  int main() {
    ///   Box<u32> var = crab::make_box<u32>(10);
    ///
    ///   some_function(crab::move(var));
    ///
    ///   // 'var' must not be used for the duration of its scope.
    ///   // the only valid use case for 'var' after this would be to let it destruct or to reassign
    ///
    ///   var = Box<u32>::from_raw(unsafe, new u32{10});
    ///
    ///   // ~Box<u32>() for var, moved-from box is freed
    /// }
    /// ```
    ///
    /// @ingroup prelude

    template<typename T>
    class Box {
      T* obj;

      static_assert(
        not crab::ty::is_const<T>,
        "Box<T> does not support const undirected types, switch "
        "Box<const T> into "
        "Box<T>"
      );

      // SizeType size;
      CRAB_INLINE constexpr explicit Box(T* const from): obj(from) {}

      /**
       * Deletes the inner content, leaving this value partially formed
       */
      CRAB_INLINE constexpr auto drop() -> void {
        if constexpr (crab::ty::array<T>) {
          delete[] obj;
        } else {
          delete obj;
        }
      }

    public:

      friend struct impl::BoxStorage<T>;

      /// Wraps pointer and assumes ownership.
      ///
      /// # Safety
      /// This function has no way of knowing if the pointer passed is actually on
      /// the heap or if something else has ownership, therefor 'unchecked' and it is
      /// the responsibility of the caller to make sure. Incorrect use of this method will result in undefined behavior.
      [[nodiscard]] CRAB_INLINE constexpr static auto from_raw(unsafe_fn, T* const ref) -> Box {
        return Box{ref};
      };

      /// Gives up ownership & opts out of RAII, giving you the raw
      /// pointer to manage yourself. (equivalent of std::unique_ptr<T>::release)
      [[nodiscard]] CRAB_INLINE constexpr auto into_raw(const SourceLocation loc = SourceLocation::current()) && -> T* {
        return mem::take(raw_ptr(loc));
      }

      /// A box cannot be empty.
      Box() = delete;

      /// A box cannot be trivially copied
      Box(const Box&) = delete;

      /// Move construction from box, leaves 'from' in an invalid state, after it is only safe to destroy or reassign.
      CRAB_INLINE constexpr Box(Box&& from) noexcept: obj{mem::take(from.obj)} {}

      /// Conversion constructor of Box<Derived> to downcast to Box<Base>
      template<std::derived_from<T> Derived>
      CRAB_INLINE constexpr Box(Box<Derived> from, const SourceLocation loc = SourceLocation::current()):
          Box{Box<Derived>::unwrap(std::move(from))} {
        crab_check_with_location(obj != nullptr, loc, "Invalid Box, moved from invalid box.");
      }

      /// Destructor of Box<T>
      CRAB_INLINE constexpr ~Box() {
        drop();
      }

      /// Implicit conversion from `Box<T>` -> `T&`, alias of as_mut.
      CRAB_INLINE constexpr operator T&() {
        return as_mut();
      }

      /// Implicit conversion from `const Box<T>` -> `const T&`, alias of as_ref.
      CRAB_INLINE constexpr operator const T&() const {
        return as_ref();
      }

      /// Implicit downcast conversion
      template<std::derived_from<T> Base>
      CRAB_INLINE constexpr operator Base&() {
        return as_ref();
      }

      /// Implicit downcast conversion
      template<std::derived_from<T> Base>
      CRAB_INLINE constexpr operator const Base&() const {
        return as_ref();
      }

      /// Implicit ref conversion
      CRAB_INLINE constexpr operator ref::Ref<T>() const {
        return as_ref();
      }

      /// Implicit ref conversion
      CRAB_INLINE constexpr operator ref::RefMut<T>() {
        return as_ref();
      }

      /// Implicit ref conversion
      template<std::derived_from<T> Base>
      CRAB_INLINE constexpr operator ref::Ref<Base>() const {
        return as_ref();
      }

      /// Implicit ref conversion
      template<std::derived_from<T> Base>
      CRAB_INLINE constexpr operator ref::RefMut<Base>() {
        return as_mut();
      }

      /// You cannot copy a box. If you wish to copy a box's inner value into a new heap allocation, consider using
      /// Box::clone.
      auto operator=(const Box&) -> void = delete;

      /// Move assignment
      CRAB_INLINE constexpr auto operator=(Box&& rhs) noexcept -> Box& {
        if (rhs.obj == obj) {
          return *this;
        }

        drop();
        obj = mem::take(rhs.obj);

        return *this;
      }

      /// Move assignment from a Box of a derived type, this will perform the required upcast
      template<std::derived_from<T> Derived>
      CRAB_INLINE constexpr auto operator=(Box<Derived>&& rhs) noexcept -> Box& {
        if (obj == ref::implicit_cast<T*>(rhs.as_ptr())) {
          return *this;
        }

        drop();
        obj = ref::implicit_cast<T*>(Box<Derived>::unwrap(std::forward<Box<Derived>>(rhs)));

        return *this;
      }

      /// Pointer access to contained type
      [[nodiscard]] CRAB_INLINE constexpr auto operator->() -> T* {
        return as_ptr_mut();
      }

      /// Pointer access to contained type
      [[nodiscard]] CRAB_INLINE constexpr auto operator->() const -> const T* {
        return as_ptr();
      }

      /// Dereference of a Box<T> leads to the inner T
      [[nodiscard]] CRAB_INLINE constexpr auto operator*() -> T& {
        return as_mut();
      }

      /// Dereference of a Box<T> leads to the inner T
      [[nodiscard]] CRAB_INLINE constexpr auto operator*() const -> const T& {
        return as_ref();
      }

      /// Stream formatter, only valid if *rhs is able to be streamed.
      CRAB_INLINE constexpr friend auto operator<<(std::ostream& os, const Box& rhs) -> std::ostream& {
        return os << *rhs;
      }

      /// Gets the inner value as a mutable pointer
      [[nodiscard]] CRAB_INLINE constexpr auto as_ptr_mut(const SourceLocation loc = SourceLocation::current()) -> T* {
        crab_dbg_check_with_location(obj != nullptr, loc, "Invalid Use of Moved Box<T>.");
        return obj;
      }

      /// Gets the inner value as a pointer
      [[nodiscard]] CRAB_INLINE constexpr auto as_ptr(const SourceLocation loc = SourceLocation::current()) const
        -> const T* {
        crab_dbg_check_with_location(obj != nullptr, loc, "Invalid Use of Moved Box<T>.");
        return obj;
      }

      /// Gets the inner value as a mutable reference
      [[nodiscard]] CRAB_INLINE constexpr auto as_mut(const SourceLocation loc = SourceLocation::current()) -> T& {
        return *as_ptr_mut(loc);
      }

      /// Gets the inner value as a reference
      [[nodiscard]]
      CRAB_INLINE constexpr auto as_ref(const SourceLocation loc = SourceLocation::current()) const -> const T& {
        return *as_ptr(loc);
      }

      ///  Performs a deep copy of the value inside, this is only possible if it valid to copy construct an instance of
      [[nodiscard]] CRAB_INLINE constexpr auto clone() const& -> Box requires ty::copy_constructible<T>
      {
        return boxed::make_box<T, const T&>(as_ref());
      }

      /// Performs a deep copy of the value inside, this is only possible if it valid to copy construct an instance of
      /// T.
      ///
      /// This will reuse the existing allocation this box contains.
      CRAB_INLINE constexpr auto clone_from(const Box& from) & -> void
        requires(ty::complete_type<T> and (ty::copy_assignable<T> or ty::copy_constructible<T>))
      {
        if constexpr (ty::copy_assignable<T>) {
          as_mut() = from;
        } else if constexpr (ty::copy_assignable<T>) {
          // destroy existing
          std::destroy_at(as_ptr_mut());

          // construct in place
          std::construct_at(as_ptr(), from.as_ref());
        }
      }

      /// Attempts to downcast the pointer
      template<std::derived_from<T> Derived>
      [[nodiscard]] CRAB_INLINE constexpr auto downcast(const SourceLocation loc = SourceLocation::current()) const
        -> opt::Option<const Derived&> {
        return ref::from_ptr(dynamic_cast<const Derived*>(as_ptr(loc)));
      }

      /// Attempts to downcast the pointer
      template<std::derived_from<T> Derived>
      [[nodiscard]] CRAB_INLINE constexpr auto downcast(const SourceLocation loc = SourceLocation::current())
        -> opt::Option<Derived&> {
        return ref::from_ptr(dynamic_cast<Derived*>(as_ptr_mut(loc)));
      }

      /// Upcasts to a base type
      template<typename Base>
      requires std::derived_from<T, Base>
      [[nodiscard]] CRAB_INLINE constexpr auto upcast() const -> const Base& {
        return as_ref();
      }

      /// Upcasts to a base type
      template<typename Base>
      requires std::derived_from<T, Base>
      [[nodiscard]] CRAB_INLINE constexpr auto upcast() -> Base& {
        return as_mut();
      }

      /// Attempts to downcast this box to another,
      /// This function will either transfer ownership to a returned Some value, or
      /// this data will be deleted entirely, invalidating this object & returning crab::none
      template<std::derived_from<T> Derived>
      [[nodiscard]] CRAB_INLINE constexpr auto downcast_lossy() && -> opt::Option<Box<Derived>> {
        auto* ptr{dynamic_cast<Derived*>(as_ptr_mut())};

        if (ptr == nullptr) {
          drop();
          return {};
        }

        obj = nullptr;
        return {Box<Derived>::from_raw(unsafe, mem::take(ptr))};
      }

    private:

      [[nodiscard]] CRAB_INLINE constexpr auto raw_ptr(const SourceLocation loc = SourceLocation::current()) -> T*& {
        crab_check_with_location(obj != nullptr, loc, "Invalid Use of Moved Box<T>.");
        return obj;
      }
    };

    /// Makes a new instance of type T on the heap with given args
    /// The values passed will be forwarded into the constructor of T
    template<ty::complete_type T, typename... Args>
    requires std::constructible_from<T, Args...>
    [[nodiscard]] CRAB_INLINE constexpr static auto make_box(Args&&... args) -> Box<T> {
      return Box<T>::from_raw(unsafe, new T(std::forward<Args>(args)...));
    }

    /// }@
  }

  using boxed::make_box;

}

namespace crab::prelude {
  using boxed::Box;
}

CRAB_PRELUDE_GUARD;

// NOLINTEND(*explicit*)

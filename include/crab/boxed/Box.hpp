// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppNonExplicitConversionOperator
#pragma once
#include <concepts>
#include <iostream>

#include <utility>

#include "crab/assertion/check.hpp"
#include "crab/core.hpp"
#include "crab/mem/take.hpp"
#include "crab/ref/ref.hpp"
#include "crab/ref/casts.hpp"
#include "crab/ref/from_ptr.hpp"

#include "crab/opt/forward.hpp"

#include "./impl/BoxStorage.hpp"

// NOLINTBEGIN(*explicit*)

namespace crab {

  namespace boxed {

    /**
     * @brief Owned Pointer (RAII) to an instance of T on the heap, this is an owned smart pointer type with no interior
     * mutability and and who is always non-null for a given valid value of Box<T>.
     *
     * A moved-from box does not leave it in a "valid but unspecified state" like other "well-behaved" types in the STL
     * (circa 2024), but leaves it in a state where it is partially-formed (only safe to reassign or destroy).
     *
     * If you are using this as a field of a class and don't want your class to be in this 'only safe to reassign or
     * destroy' state once moved, you must take on the weaker invariant and use Option<Box<T>> to explicitly allowed
     * Box<T> to be none.
     *
     * This is a replacement for std::unique_ptr, with two key differences:
     *
     * - Prevents Interior Mutability, being passed a const Box<T>& means dealing
     * with a const T&
     *
     * - A Box<T> has the invariant of always being non null, if it is then you
     * messed up with move semantics.
     */
    template<typename T>
    class Box;

    template<ty::complete_type T, typename... Args>
    requires std::constructible_from<T, Args...>
    [[nodiscard]] CRAB_INLINE constexpr static auto make_box(Args&&... args) -> Box<T>;
  }

  namespace opt {

    template<typename T>
    struct Storage;

    /**
     * @brief Storage type
     */
    template<typename T>
    struct Storage<::crab::boxed::Box<T>> final {
      using type = boxed::impl::BoxStorage<T>;
    };
  }

  namespace boxed {
    /**
     * @brief Owned Pointer (RAII) to an instance of T on the heap.
     *
     * This is a replacement for std::unique_ptr, with two key differences:
     *
     * - Prevents Interior Mutability, being passed a const Box<T>& means dealing
     * with a const T&
     * - A Box<T> has the invariant of always being non null, if it is then you
     * messed up with move semantics.
     */
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

      /**
       * @brief Wraps pointer with RAII
       *
       * This function has no way of knowing if the pointer passed is actually on
       * the heap or if something else has ownership, therefor 'unchecked' and it is
       * the responsibility of the caller to make sure.
       */
      [[nodiscard]] CRAB_INLINE constexpr static auto wrap_unchecked(T* const ref) -> Box {
        return Box{ref};
      };

      /**
       * Gives up ownership & opts out of RAII, giving you the raw
       * pointer to manage yourself. (equivalent of std::unique_ptr<T>::release
       */
      [[nodiscard]] CRAB_INLINE constexpr static auto unwrap(
        Box box,
        const SourceLocation loc = SourceLocation::current()
      ) -> T* {
        return mem::take(box.raw_ptr(loc));
      }

      Box() = delete;

      Box(const Box&) = delete;

      CRAB_INLINE constexpr Box(Box&& from) noexcept: obj{mem::take(from.obj)} {}

      template<std::derived_from<T> Derived>
      CRAB_INLINE constexpr Box(Box<Derived> from, const SourceLocation loc = SourceLocation::current()):
          Box{Box<Derived>::unwrap(std::move(from))} {
        crab_check_with_location(obj != nullptr, loc, "Invalid Box, moved from invalid box.");
      }

      /**
       * Destructor of Box<T>
       */
      CRAB_INLINE constexpr ~Box() {
        drop();
      }

      /**
       * Implicit conversion from `Box<T>` -> `T&`
       */
      CRAB_INLINE constexpr operator T&() {
        return as_mut();
      }

      /**
       * Implicit conversion from `const Box<T>` -> `const T&`
       */
      CRAB_INLINE constexpr operator const T&() const {
        return as_ref();
      }

      template<std::derived_from<T> Base>
      CRAB_INLINE constexpr operator Base&() {
        return as_ref();
      }

      template<std::derived_from<T> Base>
      CRAB_INLINE constexpr operator const Base&() const {
        return as_ref();
      }

      CRAB_INLINE constexpr operator ref::Ref<T>() const {
        return as_ref();
      }

      CRAB_INLINE constexpr operator ref::RefMut<T>() {
        return as_ref();
      }

      template<std::derived_from<T> Base>
      CRAB_INLINE constexpr operator ref::Ref<Base>() const {
        return as_ref();
      }

      template<std::derived_from<T> Base>
      CRAB_INLINE constexpr operator ref::RefMut<Base>() {
        return as_mut();
      }

      auto operator=(const Box&) -> void = delete;

      CRAB_INLINE constexpr auto operator=(Box&& rhs) noexcept -> Box& {
        if (rhs.obj == obj) {
          return *this;
        }

        drop();
        obj = mem::take(rhs.obj);

        return *this;
      }

      template<std::derived_from<T> Derived>
      CRAB_INLINE constexpr auto operator=(Box<Derived>&& rhs) noexcept -> Box& {
        if (obj == static_cast<T*>(rhs.as_ptr())) {
          return *this;
        }

        drop();
        obj = static_cast<T*>(Box<Derived>::unwrap(std::forward<Box<Derived>>(rhs)));

        return *this;
      }

      [[nodiscard]] CRAB_INLINE constexpr auto operator->() -> T* {
        return as_ptr_mut();
      }

      [[nodiscard]] CRAB_INLINE constexpr auto operator->() const -> const T* {
        return as_ptr();
      }

      [[nodiscard]] CRAB_INLINE constexpr auto operator*() -> T& {
        return as_mut();
      }

      [[nodiscard]] CRAB_INLINE constexpr auto operator*() const -> const T& {
        return as_ref();
      }

      CRAB_INLINE constexpr friend auto operator<<(std::ostream& os, const Box& rhs) -> std::ostream& {
        return os << *rhs;
      }

      [[nodiscard]] CRAB_INLINE constexpr auto as_ptr_mut(const SourceLocation loc = SourceLocation::current()) -> T* {
        crab_check_with_location(obj != nullptr, loc, "Invalid Use of Moved Box<T>.");
        return obj;
      }

      [[nodiscard]] CRAB_INLINE constexpr auto as_ptr(const SourceLocation loc = SourceLocation::current()) const
        -> const T* {
        crab_check_with_location(obj != nullptr, loc, "Invalid Use of Moved Box<T>.");
        return obj;
      }

      [[nodiscard]] CRAB_INLINE constexpr auto as_mut(const SourceLocation loc = SourceLocation::current()) -> T& {
        return *as_ptr_mut(loc);
      }

      [[nodiscard]] CRAB_INLINE constexpr auto as_ref(const SourceLocation loc = SourceLocation::current()) const
        -> const T& {
        return *as_ptr(loc);
      }

      [[nodiscard]] CRAB_INLINE constexpr auto clone() const& -> Box requires ty::copy_constructible<T>
      {
        return boxed::make_box<T, const T&>(as_ref());
      }

      CRAB_INLINE constexpr auto clone_from(const Box& from) const& -> void
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

      /**
       * @brief Attempts to downcast the pointer
       */
      template<std::derived_from<T> Derived>
      [[nodiscard]] CRAB_INLINE constexpr auto downcast(const SourceLocation loc = SourceLocation::current()) const
        -> opt::Option<const Derived&> {
        return ref::from_ptr(dynamic_cast<const Derived*>(as_ptr(loc)));
      }

      /**
       * @brief Attempts to downcast the pointer
       */
      template<std::derived_from<T> Derived>
      [[nodiscard]] CRAB_INLINE constexpr auto downcast(const SourceLocation loc = SourceLocation::current())
        -> opt::Option<Derived&> {
        return ref::from_ptr(dynamic_cast<Derived*>(as_ptr_mut(loc)));
      }

      /**
       * @brief Upcasts to a base type
       */
      template<typename Base>
      requires std::derived_from<T, Base>
      [[nodiscard]] CRAB_INLINE constexpr auto upcast() const -> const Base& {
        return as_ref();
      }

      /**
       * @brief Upcasts to a base type
       */
      template<typename Base>
      requires std::derived_from<T, Base>
      [[nodiscard]] CRAB_INLINE constexpr auto upcast() -> Base& {
        return as_mut();
      }

      /**
       * @brief Attempts to downcast this box to another,
       * This function will either transfer ownership to a returned Some value, or
       * this data will be deleted entirely, invalidating this object & returning
       * crab::none
       */
      template<std::derived_from<T> Derived>
      [[nodiscard]] CRAB_INLINE constexpr auto downcast_lossy() && -> opt::Option<Box<Derived>> {
        auto* ptr{dynamic_cast<Derived*>(as_ptr_mut())};

        if (ptr == nullptr) {
          drop();
          return {};
        }

        obj = nullptr;
        return {Box<Derived>::wrap_unchecked(mem::take(ptr))};
      }

    private:

      [[nodiscard]] CRAB_INLINE constexpr auto raw_ptr(const SourceLocation loc = SourceLocation::current()) -> T*& {
        crab_check_with_location(obj != nullptr, loc, "Invalid Use of Moved Box<T>.");
        return obj;
      }
    };

    /**
     * @brief Makes a new instance of type T on the heap with given args
     */
    template<ty::complete_type T, typename... Args>
    requires std::constructible_from<T, Args...>
    [[nodiscard]] CRAB_INLINE constexpr static auto make_box(Args&&... args) -> Box<T> {
      return Box<T>::wrap_unchecked(new T{std::forward<Args>(args)...});
    }

    /**
     * @brief Makes a new instance of type T on the heap with given args
     */
    template<typename T, typename V>
    requires std::convertible_to<T, V> and (std::integral<T> or std::floating_point<T>)
    [[nodiscard]] CRAB_INLINE constexpr static auto make_box(V&& from) -> Box<T> {
      return Box<T>::wrap_unchecked(new T{static_cast<T>(from)});
    }
  }

  using boxed::make_box;

} // namespace crab

namespace crab::prelude {
  using boxed::Box;
}

CRAB_PRELUDE_GUARD;

// NOLINTEND(*explicit*)

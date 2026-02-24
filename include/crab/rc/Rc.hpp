// NOLINTBEGIN(*-explicit-constructor)
#pragma once

#include "crab/boxed/Box.hpp"
#include "crab/ref/implicit_cast.hpp"
#include "crab/mem/take.hpp"

#include "crab/rc/impl/RcBase.hpp"
#include "crab/rc/impl/RcStorage.hpp"

/**
 * @namespace crab::rc
 * This namespace contains crab's reference-counting smart pointer types: Rc<T> and RcMut<T>, along with the
 * corresponding make_rc(_mut) constructor functions
 */
namespace crab::rc {}

namespace crab::rc {

  template<typename T>
  class Rc;

  template<typename T>
  class RcMut;

  /**
   * A reference-counted smart pointer that is a mirror to Rc<T>, with RcMut<T> being a superset of Rc<T> as the major
   * difference is RcMut allows for mutable access to the shared resource.
   */
  template<typename T>
  class RcMut final : public impl::RcBase<T, RcMut> {
    static_assert(ty::non_const<T>, "Cannot have a RcMut of a const T, consider using Rc<T>");

    using Base = impl::RcBase<T, RcMut>;

    explicit CRAB_INLINE RcMut(T* raw_owned_ptr, Base::Counter* raw_counter_ptr):
        Base{raw_owned_ptr, raw_counter_ptr} {}

  public:

    template<typename>
    friend class Rc;

    template<typename>
    friend class RcMut;

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr RcMut(const RcMut<Derived>& derived): RcMut{derived.template upcast<T>()} {}

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr RcMut(RcMut<Derived>&& derived): RcMut{mem::move(derived).template upcast<T>()} {}

    CRAB_INLINE constexpr RcMut(boxed::Box<T> from): RcMut{std::move(from).into_raw(), new Base::Counter} {}

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr auto operator=(const RcMut<Derived>& derived) -> RcMut& {
      operator=(derived.template upcast<T>());
      return *this;
    }

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr auto operator=(RcMut<Derived>&& derived) -> RcMut& {
      operator=(mem::move(derived).template upcast<T>());
      return *this;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto operator->() const -> T* {
      return as_ptr_mut();
    }

    [[nodiscard]] CRAB_INLINE constexpr auto operator*() const -> T& {
      return as_mut();
    }

    [[nodiscard]] CRAB_INLINE constexpr operator T&() const {
      return as_mut();
    }

    [[nodiscard]] CRAB_INLINE constexpr operator T*() const {
      return as_ptr_mut();
    }

    [[nodiscard]] constexpr auto as_ptr_mut() const -> T* {
      this->assert_valid();
      return this->data;
    }

    /**
     * Returns a mutable reference to the value inside
     */
    [[nodiscard]] constexpr auto as_mut() const -> T& {
      this->assert_valid();
      return *this->data;
    }

    /**
     * Returns a copy of this RcMut
     */
    [[nodiscard]] auto clone() const -> RcMut {
      return *this;
    }

    /**
     * Unsafe factory method that takes in an *unmanaged* pointer (+ possibly an existing counter) to construct
     * shared ownership around it.
     */
    [[nodiscard]]
    static constexpr auto from_owned_unchecked(T* data_ptr, Base::Counter* counter_ptr = new Base::Counter) -> RcMut {
      return RcMut{data_ptr, counter_ptr};
    }

    using Base::get_ref_count;
    using Base::get_weak_ref_count;
    using Base::is_unique;
    using Base::as_ptr;
    using Base::as_ref;
    using Base::is_valid;
    using Base::upcast;
    using Base::downcast;
  };

  template<typename T>
  class Rc final : public impl::RcBase<const T, Rc> {
    static_assert(ty::non_const<T>);

    using Base = impl::RcBase<const T, Rc>;

    explicit CRAB_INLINE Rc(const T* data_ptr, Base::Counter* counter_ptr): Base{data_ptr, counter_ptr} {}

  public:

    template<typename>
    friend class Rc;

    template<typename>
    friend class RcMut;

    CRAB_INLINE constexpr Rc(RcMut<T>&& from):
        Base{
          mem::take(from.data),
          mem::take(from.counter),
        } {
      crab_check(this->is_valid(), "Cannot move from a moved-from RcMut");
    }

    CRAB_INLINE constexpr Rc(const RcMut<T>& from): Rc{reinterpret_cast<const Rc&>(from)} {
      // SAFETY: reinterpret_cast is valid here, forall T: Rc<T> has the exact same layout and data as RcMut<T>
    }

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr Rc(const RcMut<Derived>& derived): Rc{derived.template upcast<T>()} {}

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr Rc(RcMut<Derived>&& derived): Rc{mem::move(derived).template upcast<T>()} {}

    CRAB_INLINE constexpr Rc(boxed::Box<T> from): Rc{std::move(from).into_raw(), new Base::Counter} {}

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr auto operator=(const RcMut<Derived>& derived) -> Rc& {
      operator=(implicit_cast<const Rc&>(derived));
      return *this;
    }

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr auto operator=(RcMut<Derived>&& derived) -> Rc& {
      operator=(implicit_cast<Rc&&>(mem::move(derived)));
      return *this;
    }

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr Rc(const Rc<Derived>& derived): Rc{derived.template upcast<T>()} {}

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr Rc(Rc<Derived>&& derived): Rc{mem::move(derived).template upcast<T>()} {}

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr auto operator=(const Rc<Derived>& derived) -> Rc& {
      operator=(derived.template upcast<T>());
      return *this;
    }

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr auto operator=(Rc<Derived>&& derived) -> Rc& {
      operator=(mem::move(derived).template upcast<T>());
      return *this;
    }

    constexpr auto operator=(RcMut<T>&& from) -> Rc& {
      crab_check(from.is_valid(), "Cannot move from partially constructed RcMut");

      if (this->counter == from.counter) [[unlikely]] {
        return *this;
      }

      // destruct old counter & data
      this->destroy();

      // destruct old counter & data
      this->data = mem::take(from.data);
      this->counter = mem::take(from.data);

      return *this;
    }

    /**
     * Copy assignment operator from an RcMut of the same type
     */
    constexpr auto operator=(const RcMut<T>& from) -> Rc& {
      // SAFETY: reinterpret_cast is valid here, forall T: Rc<T> has the exact same layout and data as RcMut<T>
      operator=(reinterpret_cast<const Rc&>(from));
      return *this;
    }

    /**
     * Conversion operator that is any alias to Rc::as_ref
     */
    [[nodiscard]] CRAB_INLINE constexpr operator const T&() const {
      return this->as_ref();
    }

    /**
     * Conversion operator that is any alias to Rc::as_ptr
     */
    [[nodiscard]] CRAB_INLINE constexpr operator const T*() const {
      return this->as_ptr();
    }

    /**
     * Returns a copy of this Rc
     */
    [[nodiscard]] auto clone() const -> Rc {
      return *this;
    }

    /**
     * Unsafe factory method that takes in an *unmanaged* pointer (+ possibly an existing counter) to construct
     * shared ownership around it.
     */
    [[nodiscard]] static constexpr auto from_owned_unchecked(
      const T* owned_ptr,
      Base::Counter* counter_ptr = new Base::Counter
    ) -> Rc {
      return Rc{owned_ptr, counter_ptr};
    }

    using Base::get_ref_count;

    using Base::get_weak_ref_count;

    using Base::is_unique;

    using Base::operator->;

    using Base::operator*;

    using Base::as_ptr;

    using Base::as_ref;

    using Base::is_valid;

    using Base::upcast;

    using Base::downcast;
  };

  /// Specialization for fmt to be able to format a Rc<T> if T is formattable.
  template<typename T>
  [[nodiscard]] auto format_as(const Rc<T>& rc) -> const T& {
    return rc.as_ref();
  }

  /// Specialization for fmt to be able to format a RcMut<T> if T is formattable.
  template<typename T>
  [[nodiscard]] auto format_as(const RcMut<T>& rc) -> const T& {
    return rc.as_ref();
  }

  // Constructs the type T from the given arguments and creates an Rc around it. Note unlike RcMut, an Rc can never be
  // converted into mutable safely.
  template<ty::non_const T, typename... Args>
  [[nodiscard]] constexpr auto make_rc(Args&&... args) -> Rc<T> {
    static_assert(std::constructible_from<T, Args...>, "Cannot construct type from the given arguments");
    return Rc<T>::from_owned_unchecked(new T(std::forward<Args>(args)...));
  }

  // Constructs the type T from the given arguments and creates an RcMut around it
  template<ty::non_const T, typename... Args>
  requires std::constructible_from<T, Args...>
  [[nodiscard]] constexpr auto make_rc_mut(Args&&... args) -> RcMut<T> {
    static_assert(std::constructible_from<T, Args...>, "Cannot construct type from the given arguments");

    return RcMut<T>::from_owned_unchecked(new T(std::forward<Args>(args)...));
  }

}

namespace crab {
  using ::crab::rc::make_rc;
  using ::crab::rc::make_rc_mut;

  namespace prelude {
    using ::crab::rc::Rc;
    using ::crab::rc::RcMut;
  }
}

CRAB_PRELUDE_GUARD;

// NOLINTEND(*-explicit-constructor)

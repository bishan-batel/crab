// NOLINTBEGIN(*-explicit-constructor)
#pragma once

#include "crab/casts.hpp"
#include "crab/mem/take.hpp"
#include "crab/type_traits.hpp"

#include "crab/rc/impl/Counter.hpp"
#include "crab/rc/impl/RcBase.hpp"
#include "crab/rc/impl/RcStorage.hpp"

namespace crab::rc {

  template<typename T, bool IsAtomic = false>
  class Rc;

  template<typename T, bool IsAtomic = false>
  class RcMut;

  template<typename T, bool IsAtomic>
  class RcMut final : public impl::RcBase<T, RcMut, IsAtomic> {
    static_assert(ty::non_const<T>);

    using Base = impl::RcBase<T, RcMut, IsAtomic>;

    explicit CRAB_INLINE RcMut(T* raw_owned_ptr, Base::Counter* raw_counter_ptr):
        Base{raw_owned_ptr, raw_counter_ptr} {}

  public:

    template<typename, bool>
    friend class Rc;

    template<typename, bool>
    friend class RcMut;

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr RcMut(const RcMut<Derived>& derived): RcMut{derived.template upcast<T>()} {}

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr RcMut(RcMut<Derived>&& derived): RcMut{mem::move(derived).template upcast<T>()} {}

    CRAB_INLINE constexpr RcMut(boxed::Box<T> from): RcMut{boxed::Box<T>::unwrap(std::move(from)), new Base::Counter} {}

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

    [[nodiscard]] constexpr auto as_mut() const -> T& {
      this->assert_valid();
      return *this->data;
    }

    [[nodiscard]] auto clone() const -> RcMut {
      return *this;
    }

    [[nodiscard]]
    static constexpr auto from_owned_unchecked(T* data_ptr, Base::Counter* counter_ptr = new Base::Counter) -> RcMut {
      return RcMut{data_ptr, counter_ptr};
    }
  };

  template<typename T, bool IsAtomic>
  class Rc final : public impl::RcBase<const T, Rc, IsAtomic> {
    static_assert(ty::non_const<T>);

    using Base = impl::RcBase<const T, Rc, IsAtomic>;

    explicit CRAB_INLINE Rc(const T* data_ptr, Base::Counter* counter_ptr): Base{data_ptr, counter_ptr} {}

  public:

    template<typename, bool>
    friend class Rc;

    template<typename, bool>
    friend class RcMut;

    CRAB_INLINE constexpr Rc(RcMut<T, IsAtomic>&& from):
        Base{
          mem::take(from.data),
          mem::take(from.counter),
        } {
      debug_assert(this->is_valid(), "Cannot move from a moved-from RcMut");
    }

    CRAB_INLINE constexpr Rc(const RcMut<T, IsAtomic>& from): Rc{reinterpret_cast<const Rc&>(from)} {
      // SAFETY: reinterpret_cast is valid here, forall T: Rc<T> has the exact same layout and data as RcMut<T>
    }

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr Rc(const RcMut<Derived>& derived): Rc{derived.template upcast<T>()} {}

    template<std::derived_from<T> Derived>
    CRAB_INLINE constexpr Rc(RcMut<Derived>&& derived): Rc{mem::move(derived).template upcast<T>()} {}

    CRAB_INLINE constexpr Rc(boxed::Box<T> from): Rc{boxed::Box<T>::unwrap(std::move(from)), new Base::Counter} {}

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

    constexpr auto operator=(RcMut<T, IsAtomic>&& from) -> Rc& {
      debug_assert(from.is_valid(), "Cannot move from partially constructed RcMut");

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

    constexpr auto operator=(const RcMut<T, IsAtomic>& from) -> Rc& {
      // SAFETY: reinterpret_cast is valid here, forall T: Rc<T> has the exact same layout and data as RcMut<T>
      operator=(reinterpret_cast<const Rc&>(from));
      return *this;
    }

    [[nodiscard]] CRAB_INLINE constexpr operator const T&() const {
      return this->as_ref();
    }

    [[nodiscard]] CRAB_INLINE constexpr operator const T*() const {
      return this->as_ptr();
    }

    [[nodiscard]] auto clone() const -> Rc {
      return *this;
    }

    [[nodiscard]] static constexpr auto from_owned_unchecked(
      const T* owned_ptr,
      Base::Counter* counter_ptr = new Base::Counter
    ) -> Rc {
      return Rc{owned_ptr, counter_ptr};
    }
  };

  template<ty::non_const T, typename... Args>
  requires std::constructible_from<T, Args...>
  [[nodiscard]] constexpr auto make_rc(Args&&... args) -> Rc<T> {
    return Rc<T>::from_owned_unchecked(new T{std::forward<Args>(args)...});
  }

  template<ty::non_const T, typename... Args>
  requires std::constructible_from<T, Args...>
  [[nodiscard]] constexpr auto make_rc_mut(Args&&... args) -> RcMut<T> {
    return RcMut<T>::from_owned_unchecked(new T{std::forward<Args>(args)...});
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

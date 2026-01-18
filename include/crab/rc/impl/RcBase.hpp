
#pragma once

#include "Counter.hpp"
#include "crab/assertion/check.hpp"
#include "crab/mem/address_of.hpp"
#include "crab/mem/take.hpp"
#include "crab/type_traits.hpp"

namespace crab::rc::impl {
  template<typename T, template<typename, bool> class Self, bool IsAtomic = false>
  class RcBase {
    static_assert(ty::non_reference<T>, "Cannot allocate data for a reference type (T& / const T&)");

  public:

    using Counter = impl::Counter<IsAtomic>;
    using TMut = std::remove_const_t<T>;
    using TConst = std::add_const_t<TMut>;

  protected:

    explicit CRAB_INLINE RcBase(T* raw_owned_ptr, Counter* raw_counter_ptr):
        data{raw_owned_ptr}, counter{raw_counter_ptr} {}

    auto destroy() {

      if (counter == nullptr) [[unlikely]] {
        crab_check(data == nullptr, "No counter paired with data");
        return;
      }

      crab_check(data != nullptr, "No data paired with counter");

      // if decrementing the strong counter leaves it 0 and the counter has 0 weak references, delete it
      if (counter->decrement_strong() and not counter->has_any_weak()) {
        delete counter;
      }

      data = nullptr;
      counter = nullptr;
    }

  public:

    RcBase(const RcBase& from): data{from.data}, counter{from.counter} {
      crab_check(from.is_valid(), "Cannot copy from a moved-from RcBase");
      counter->increment_strong();
    }

    RcBase(RcBase&& from) noexcept: data{mem::take(from.data)}, counter{mem::take(from.counter)} {}

    constexpr RcBase& operator=(const RcBase& from) {
      if (mem::address_of(from) == this) [[unlikely]] {
        return *this;
      }

      crab_check(from.is_valid(), "Cannot copy from a partially constructed RcBase");

      // if we are copying from another rcbase of the same counter, we can do nothing
      if (counter == from.counter) [[unlikely]] {
        return *this;
      }

      // destruct old counter & data
      destroy();

      // copy new values
      data = from.data;
      counter = from.counter;

      // inform counter that there is a new copy
      counter->increment_strong();

      return *this;
    }

    constexpr RcBase& operator=(RcBase&& from) noexcept {
      if (mem::address_of(from) == this) [[unlikely]] {
        return *this;
      }

      // if we are being assigned the same counter, we can just let it destruct and keep ours
      if (counter == from.counter) [[unlikely]] {
        return *this;
      }

      // destruct old counter & data
      destroy();

      // take new values
      data = mem::take(from.data);
      counter = mem::take(from.counter);

      // no need to update any count

      return *this;
    }

    ~RcBase() {
      destroy();
    }

    [[nodiscard]] constexpr auto get_ref_count() const -> usize {
      if (counter == nullptr) [[unlikely]] {
        return 0;
      }

      return counter->strong_count();
    }

    [[nodiscard]] constexpr auto get_ref_count_unchecked() const -> usize {
      crab_dbg_check(counter != nullptr, "canot read from null counter");

      return counter->strong_count();
    }

    [[nodiscard]] constexpr auto get_weak_ref_count() const -> usize {
      if (counter == nullptr) [[unlikely]] {
        return 0;
      }

      return counter->weak_count();
    }

    [[nodiscard]] constexpr auto get_weak_ref_count_unchecked() const -> usize {
      crab_dbg_check(counter != nullptr, "canot read from null counter");
      return counter->weak_count();
    }

    [[nodiscard]] constexpr auto is_unique() const -> bool {
      return get_ref_count() == 1;
    }

    [[nodiscard]] constexpr auto is_unique_unchecked() const -> bool {
      return get_ref_count_unchecked() == 1;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto as_ptr() const -> TConst* {
      assert_valid();
      return data;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto as_ref() const -> TConst& {
      assert_valid();
      return *data;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto operator->() const -> TConst* {
      return as_ptr();
    }

    [[nodiscard]] CRAB_INLINE constexpr auto operator*() const -> TConst& {
      return as_ref();
    }

    [[nodiscard]] constexpr auto is_valid() const -> bool {
      return data != nullptr and counter != nullptr;
    }

    template<typename U>
    requires ty::non_const<U> and std::derived_from<T, U>
    [[nodiscard]] auto upcast() const& -> Self<U, IsAtomic> {
      return Self<TMut, IsAtomic>{reinterpret_cast<const Self<TMut, IsAtomic>&>(*this)}.template upcast<U>();
    }

    template<typename U>
    requires ty::non_const<U> and std::derived_from<T, U>
    [[nodiscard]] auto upcast() && -> Self<U, IsAtomic> {
      assert_valid();

      return {
        Self<U, IsAtomic>::from_owned_unchecked(mem::take(data), mem::take(counter)),
      };
    }

    template<std::derived_from<T> U>
    requires ty::non_const<U>
    [[nodiscard]] auto downcast() const& -> opt::Option<Self<U, IsAtomic>> {
      return Self<TMut, IsAtomic>{reinterpret_cast<const Self<TMut, IsAtomic>&>(*this)}.template downcast<U>();
    }

    template<std::derived_from<T> U>
    requires ty::non_const<U>
    [[nodiscard]] auto downcast() && -> opt::Option<Self<U, IsAtomic>> {
      assert_valid();

      auto* casted = dynamic_cast<ty::conditional<ty::is_const<T>, const U, U>*>(data);

      if (casted == nullptr) {
        return {};
      }

      counter->increment_strong();

      return {Self<U, IsAtomic>::from_owned_unchecked(casted, counter)};
    }

  protected:

    CRAB_INLINE constexpr auto assert_valid(const SourceLocation& loc = SourceLocation::current()) const {
      crab_check_with_location(
        is_valid(),
        loc,
        "Invalid use of partially constructed Rc/RcMut (most likely an use after move)"
      );
    }

    T* data{nullptr};
    Counter* counter{nullptr};
  };
}

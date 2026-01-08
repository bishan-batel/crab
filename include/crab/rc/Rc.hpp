#pragma once

#include <atomic>
#include "crab/mem/address_of.hpp"
#include "crab/mem/take.hpp"
#include "crab/preamble.hpp"
#include "crab/type_traits.hpp"
#include "crab/debug.hpp"

namespace crab::experimental::rc {

  namespace impl {

    template<bool is_thread_safe>
    class Counter {

    public:

      explicit Counter(usize strong_count = 1, usize weak_count = 0): strong{strong_count}, weak{weak_count} {}

      constexpr auto increment_strong() -> void {
        strong++;
      }

      constexpr auto increment_weak() -> void {
        weak++;
      }

      [[nodiscard]] constexpr auto has_any_strong() const -> bool {
        return strong != 0;
      }

      [[nodiscard]] constexpr auto has_any_weak() const -> bool {
        return weak != 0;
      }

      [[nodiscard]] constexpr auto strong_count() const -> usize {
        return strong;
      }

      [[nodiscard]] constexpr auto weak_count() const -> usize {
        return weak;
      }

      /**
       * Decrements the strong reference counter
       *
       * Returns whether this leaves the counter at 0
       */
      [[nodiscard]] constexpr auto decrement_strong(const SourceLocation& loc = SourceLocation::current()) -> bool {
        debug_assert_transparent(strong != 0, loc, "Counter::decrement should not cause unsigned underflow");

        return --strong == 0;
      }

      /**
       * Decrements the weak reference counter
       *
       * Returns whether this leaves the counter at 0
       */
      [[nodiscard]] constexpr auto decrement_weak(const SourceLocation& loc = SourceLocation::current()) -> bool {
        debug_assert_transparent(strong != 0, loc, "Counter::decrement should not cause unsigned underflow");

        return --weak == 0;
      }

    private:

      ty::conditional<is_thread_safe, std::atomic_size_t, usize> strong{}, weak{};
    };

    template<typename T, bool is_thread_safe = false>
    class RcBase {
      static_assert(ty::non_reference<T>, "Cannot allocate data for a reference type (T& / const T&)");

    public:

      using Counter = impl::Counter<is_thread_safe>;
      using TMut = std::remove_const_t<T>;
      using TConst = std::add_const_t<TMut>;

    protected:

      explicit CRAB_INLINE RcBase(T* raw_owned_ptr, Counter* raw_counter_ptr):
          data{raw_owned_ptr}, counter{raw_counter_ptr} {}

      auto destroy() {

        if (counter == nullptr) [[unlikely]] {
          debug_assert(data == nullptr, "No counter paired with data");
          return;
        }

        debug_assert(data != nullptr, "No data paired with counter");

        // if decrementing the strong counter leaves it 0 and the counter has 0 weak references, delete it
        if (counter->decrement_strong() and not counter->has_any_weak()) {
          delete counter;
        }

        data = nullptr;
        counter = nullptr;
      }

    public:

      RcBase(const RcBase& from): data{from.data}, counter{from.counter} {
        debug_assert(from.is_valid(), "Cannot copy from a moved-from RcBase");
        counter->increment_strong();
      }

      RcBase(RcBase&& from) noexcept: data{mem::take(from.data)}, counter{mem::take(from.counter)} {
        debug_assert(is_valid(), "Cannot move from a moved-from RcBase");
      }

      constexpr RcBase& operator=(const RcBase& from) {
        if (mem::address_of(from) == this) [[unlikely]] {
          return *this;
        }

        debug_assert(from.is_valid(), "Cannot copy from a partially constructed RcBase");

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

        debug_assert(from.is_valid(), "Cannot move from a partially constructed RcBase");

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
        debug_assert(counter != nullptr, "canot read from null counter");

        return counter->strong_count();
      }

      [[nodiscard]] constexpr auto get_weak_ref_count() const -> usize {
        if (counter == nullptr) [[unlikely]] {
          return 0;
        }

        return counter->weak_count();
      }

      [[nodiscard]] constexpr auto get_weak_ref_count_unchecked() const -> usize {
        debug_assert(counter != nullptr, "canot read from null counter");
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

    protected:

      CRAB_INLINE constexpr auto assert_valid(const SourceLocation& loc = SourceLocation::current()) const {
        debug_assert_transparent(
          is_valid(),
          loc,
          "Invalid use of partially constructed Rc/RcMut (most likely an use after move)"
        );
      }

      T* data{nullptr};
      Counter* counter{nullptr};
    };
  }

  template<typename T, bool is_thread_safe = false>
  class Rc;

  template<typename T, bool is_thread_safe = false>
  class RcMut;

  template<typename T, bool is_thread_safe>
  class RcMut final : public impl::RcBase<T, is_thread_safe> {
    static_assert(ty::non_const<T>);

    using Base = impl::RcBase<T>;

    // using Counter = impl::RcBase<T>::Counter;

    explicit CRAB_INLINE RcMut(T* raw_owned_ptr, Base::Counter* raw_counter_ptr):
        Base{raw_owned_ptr, raw_counter_ptr} {}

  public:

    template<typename, bool>
    friend class Rc;

    template<typename, bool>
    friend class RcMut;

    [[nodiscard]] CRAB_INLINE constexpr auto operator->() -> T* {
      return as_ptr_mut();
    }

    [[nodiscard]] CRAB_INLINE constexpr auto operator*() -> T& {
      return as_mut();
    }

    [[nodiscard]] constexpr auto as_ptr_mut() -> T* {
      this->assert_valid();
      return this->data;
    }

    [[nodiscard]] constexpr auto as_mut() -> T& {
      this->assert_valid();
      return *this->data;
    }

    [[nodiscard]]
    static constexpr auto from_owned_unchecked(T* data_ptr, Base::Counter* counter_ptr = new Base::Counter) -> RcMut {
      return RcMut{data_ptr, counter_ptr};
    }
  };

  template<typename T, bool is_thread_safe>
  class Rc final : public impl::RcBase<const T> {
    static_assert(ty::non_const<T>);

    using Base = impl::RcBase<const T>;

    explicit CRAB_INLINE Rc(T* data_ptr, Base::Counter* counter_ptr): Base{data_ptr, counter_ptr} {}

  public:

    template<typename, bool>
    friend class Rc;

    template<typename, bool>
    friend class RcMut;

    [[nodiscard]]
    static constexpr auto from_owned_unchecked(T* raw_owned_ptr, Base::Counter* raw_counter_ptr = new Base::Counter)
      -> Rc {
      return Rc{raw_owned_ptr, raw_counter_ptr};
    }
  };

  template<typename T, typename... Args>
  requires std::constructible_from<T, Args...>
  [[nodiscard]] auto make_rc(Args&&... args) -> Rc<T> {
    return Rc<T>::from_owned_unchecked(new T{std::forward<Args>(args)...});
  }

  /**
   * Creates a new mutable reference counted instance of 'const T'
   * @tparam T The type to be heap allocated & reference counted
   * @tparam Args Argument types to be passed to T's constructor
   * @param args Arguments to be passed to T's constructor
   */
  template<typename T, typename... Args>
  requires std::constructible_from<T, Args...>
  [[nodiscard]] auto make_rc_mut(Args&&... args) -> RcMut<T> {
    return RcMut<T>::from_owned_unchecked(new T{std::forward<Args>(args)...});
  }

  namespace prelude {
    using rc::Rc;
    using rc::RcMut;
    using rc::make_rc;
    using rc::make_rc_mut;
  }
}

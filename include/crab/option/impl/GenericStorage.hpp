#pragma once

#include "crab/core.hpp"
#include "crab/mem/move.hpp"
#include "crab/mem/address_of.hpp"
#include "crab/mem/forward.hpp"
#include "crab/mem/size_of.hpp"
#include "crab/option/none.hpp"
#include "crab/type_traits.hpp"

namespace crab::opt::impl {
  /**
   * Generic tagged union storage for Option<T>
   */
  template<typename T>
  struct GenericStorage {

    /**
     * Initialise to Some(value)
     */
    CRAB_INLINE_CONSTEXPR explicit GenericStorage(T&& value): in_use_flag{true} {
      std::construct_at<T, T&&>(address(), mem::forward<T>(value));
    }

    /**
     * Copy initialise to Some(value)
     */
    CRAB_INLINE_CONSTEXPR explicit GenericStorage(const T& value) requires ty::copy_constructible<T>
        : in_use_flag(true) {
      std::construct_at<T, const T&>(address(), value);
    }

    /**
     * Default initialises to none
     */
    CRAB_INLINE_CONSTEXPR explicit GenericStorage(const None& = {}): in_use_flag{false} {}

    constexpr GenericStorage(const GenericStorage& from) requires ty::copy_constructible<T>
        : in_use_flag{from.in_use_flag} {
      if (in_use()) {
        std::construct_at<T, const T&>(address(), from.value());
      }
    }

    constexpr GenericStorage(GenericStorage&& from) noexcept: in_use_flag{from.in_use_flag} {
      if (in_use()) {
        std::construct_at<T, T&&>(address(), mem::move(from.value()));
        std::destroy_at(from.address());
        from.in_use_flag = false;
      }
    }

    constexpr GenericStorage& operator=(const GenericStorage& from) {
      if (crab::mem::address_of(from) == this) {
        return *this;
      }

      if (from.in_use()) {
        operator=(from.value());
      } else {
        operator=(None{});
        return *this;
      }

      return *this;
    }

    constexpr GenericStorage& operator=(GenericStorage&& from) noexcept(std::is_nothrow_move_assignable_v<T>) {
      if (not from.in_use()) {
        operator=(None{});
        return *this;
      }

      if (in_use_flag) {
        *address() = mem::move(from.value());
      } else {
        std::construct_at<T, T&&>(address(), mem::move(from.value()));
        in_use_flag = true;
      }

      std::destroy_at(from.address());
      from.in_use_flag = false;

      return *this;
    }

    constexpr ~GenericStorage() {

      if (in_use_flag) {
        std::destroy_at(address());
        in_use_flag = false;
      }
    }

    /**
     * Move reassign to Some(value)
     */
    constexpr auto operator=(T&& value) noexcept(std::is_nothrow_move_assignable_v<T>) -> GenericStorage& {
      if (in_use_flag) {
        *address() = mem::forward<T>(value);
      } else {
        std::construct_at<T, T&&>(address(), mem::forward<T>(value));
        in_use_flag = true;
      }
      return *this;
    }

    /**
     * Copy reassign to Some(value)
     */
    constexpr auto operator=(const T& from) -> GenericStorage& requires ty::copy_assignable<T>
    {
      if (in_use_flag) {
        value() = from;
      } else {
        std::construct_at<T, const T&>(address(), from);
        in_use_flag = true;
      }
      return *this;
    }

    /**
     * Reassign to None
     */
    constexpr auto operator=(const None&) -> GenericStorage& {

      if (in_use_flag) {
        std::destroy_at(address());
        in_use_flag = false;
      }

      return *this;
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto value() const& -> const T& {
      return reinterpret_cast<const T&>(bytes);
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto value() & -> T& {
      return reinterpret_cast<T&>(bytes);
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto value() && -> T {
#if CRAB_GCC_VERSION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
      T moved{mem::move(reinterpret_cast<T&>(bytes))};

#if CRAB_GCC_VERSION
#pragma GCC diagnostic pop
#endif
      std::destroy_at<T>(address());
      in_use_flag = false;
      return moved;
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto in_use() const -> bool {
      return in_use_flag;
    }

  private:

    CRAB_NODISCARD_INLINE_CONSTEXPR CRAB_RETURNS_NONNULL auto address() -> T* {
      return reinterpret_cast<T*>(bytes.data());
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR CRAB_RETURNS_NONNULL auto address() const -> const T* {
      return reinterpret_cast<const T*>(bytes.data());
    }

    alignas(T) SizedArray<std::byte, mem::size_of<T>()> bytes;
    bool in_use_flag;
  };
}

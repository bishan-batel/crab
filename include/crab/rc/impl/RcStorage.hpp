#pragma once

#include "crab/core.hpp"
#include "crab/core/discard.hpp"
#include "crab/mem/move.hpp"
#include "crab/opt/none.hpp"

namespace crab {

  namespace rc {
    template<typename T>
    class Rc;

    template<typename T>
    class RcMut;

    namespace impl {
      template<typename RefCounted>
      struct RcStorage final {

        CRAB_INLINE constexpr explicit RcStorage(RefCounted value): inner{mem::move(value)} {}

        CRAB_INLINE constexpr explicit RcStorage(const opt::None& = {}):
            inner{RefCounted::from_owned_unchecked(nullptr, nullptr)} {}

        CRAB_INLINE constexpr RcStorage(const RcStorage& storage):
            inner{storage.in_use() ? storage.inner : RefCounted::from_owned_unchecked(nullptr, nullptr)} {}

        CRAB_INLINE constexpr RcStorage(RcStorage&& storage):
            inner{storage.in_use() ? storage.inner : RefCounted::from_owned_unchecked(nullptr, nullptr)} {}

        CRAB_INLINE constexpr auto operator=(const RcStorage& value) -> RcStorage& {
          if (&value == this) [[unlikely]] {
            return *this;
          }

          if (value.in_use()) {
            inner = value.inner;
          } else {
            operator=(none);
          }

          return *this;
        }

        CRAB_INLINE constexpr auto operator=(RcStorage&& value) -> RcStorage& {
          if (&value == this) [[unlikely]] {
            return *this;
          }

          if (value.in_use()) {
            inner = mem::move(value.inner);
          } else {
            operator=(none);
          }

          return *this;
        }

        CRAB_INLINE constexpr auto operator=(RefCounted&& value) -> RcStorage& {
          inner = mem::move(value);
          return *this;
        }

        CRAB_INLINE constexpr auto operator=(const opt::None&) -> RcStorage& {
          if (in_use()) {
            crab::discard(RefCounted{mem::move(inner)});
          }
          return *this;
        }

        [[nodiscard]] CRAB_INLINE constexpr auto value() const& -> const RefCounted& {
          return inner;
        }

        [[nodiscard]] CRAB_INLINE constexpr auto value() & -> RefCounted& {
          return inner;
        }

        [[nodiscard]] CRAB_INLINE constexpr auto value() && -> RefCounted {
          return mem::move(inner);
        }

        [[nodiscard]] CRAB_INLINE constexpr auto in_use() const -> bool {
          return inner.is_valid();
        }

      private:

        RefCounted inner;
      };
    }
  }

  namespace opt {
    template<typename T>
    struct Storage;

    template<typename T>
    struct Storage<rc::Rc<T>> final {
      using type = rc::impl::RcStorage<rc::Rc<T>>;
    };

    template<typename T>
    struct Storage<rc::RcMut<T>> final {
      using type = rc::impl::RcStorage<rc::RcMut<T>>;
    };
  }
}

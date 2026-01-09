#pragma once

#include "crab/core.hpp"
#include "crab/mem/move.hpp"
#include "crab/opt/none.hpp"

namespace crab {

  namespace rc {
    template<typename, bool>
    class Rc;

    template<typename, bool>
    class RcMut;

    namespace impl {
      template<typename RefCounted>
      struct RcStorage final {

        CRAB_INLINE_CONSTEXPR explicit RcStorage(RefCounted value): inner{mem::move(value)} {}

        CRAB_INLINE_CONSTEXPR explicit RcStorage(const opt::None& = {}):
            inner{RefCounted::from_owned_unchecked(nullptr, nullptr)} {}

        CRAB_INLINE_CONSTEXPR auto operator=(RefCounted&& value) -> RcStorage& {
          inner = mem::move(value);
          return *this;
        }

        CRAB_INLINE_CONSTEXPR auto operator=(const opt::None&) -> RcStorage& {
          if (in_use()) {
            crab::discard(RefCounted{mem::move(inner)});
          }
          return *this;
        }

        CRAB_NODISCARD_INLINE_CONSTEXPR auto value() const& -> const RefCounted& {
          return inner;
        }

        CRAB_NODISCARD_INLINE_CONSTEXPR auto value() & -> RefCounted& {
          return inner;
        }

        CRAB_NODISCARD_INLINE_CONSTEXPR auto value() && -> RefCounted {
          return mem::move(inner);
        }

        CRAB_NODISCARD_INLINE_CONSTEXPR auto in_use() const -> bool {
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

    template<typename T, bool IsAtomic>
    struct Storage<rc::Rc<T, IsAtomic>> final {
      using type = rc::impl::RcStorage<rc::Rc<T, IsAtomic>>;
    };

    template<typename T, bool IsAtomic>
    struct Storage<rc::RcMut<T, IsAtomic>> final {
      using type = rc::impl::RcStorage<rc::RcMut<T, IsAtomic>>;
    };
  }
}

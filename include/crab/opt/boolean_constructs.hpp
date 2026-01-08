#include "crab/core.hpp"
#include "crab/mem/forward.hpp"
#include "crab/mem/move.hpp"
#include "crab/opt/Option.hpp"
#include "crab/type_traits.hpp"

namespace crab::opt {
  /**
   * Maps a boolean to an option if it is true
   */
  template<ty::provider F>
  [[nodiscard]] CRAB_INLINE constexpr auto then(const bool cond, F&& func) {
    using Return = Option<crab::ty::functor_result<F>>;

    if (not cond) {
      return Return{};
    }

    return Return{std::invoke(func)};
  }

  /**
   * Maps a boolean to an option if it is false
   */
  template<ty::provider F>
  [[nodiscard]] CRAB_INLINE constexpr auto unless(const bool cond, F&& func) {
    using Return = Option<ty::functor_result<F>>;

    if (cond) {
      return Return{};
    }

    return Return{std::invoke(func)};
  }

  /**
   * Consumes given option and returns the contained value, will throw
   * if none found
   * @param from Option to consume
   */
  template<typename T>
  [[nodiscard]] CRAB_INLINE constexpr auto unwrap(Option<T>&& from) -> T {
    return mem::forward<T>(from).unwrap();
  }
}

namespace crab {
  using opt::then;
  using opt::unless;
}

#include "./preamble.hpp"

namespace choice {

  template<typename... T>
  class Choice<T...> {
    static_assert();
  };
}

#if CRAB_USE_PRELUDE

using choice::Choice;

#endif

/// @file crab/num/floating.hpp
/// @ingroup num

#pragma once

#include "crab/core.hpp"

/// @addtogroup num
/// @{

namespace crab::num {
  /// 32 Bit Floating Point Number
  using f32 = float;

  /// 64 Bit Floating Point Number
  using f64 = long double;
}

/// }@

namespace crab {
  using namespace crab::num;

  namespace prelude {
    using namespace crab::num;
  }
}

CRAB_PRELUDE_GUARD;

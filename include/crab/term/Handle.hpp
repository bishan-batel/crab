///@file crab/term/Handle.hpp

#pragma once

namespace crab::term {

  /// Enumeration for the different standard file IO's given to a process for terminal interactions.
  enum class Handle {
    /// stdout / std::cout
    Out,

    /// stderr / std::cerr
    Error,

    /// stdin / std::cin
    Input,
  };

}

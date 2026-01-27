#pragma once

namespace crab::term {
  enum class Handle {
    /**
     * stdout
     */
    Out,

    /**
     * stderr
     */
    Error,

    /**
     * stdin
     */
    Input,
  };
}

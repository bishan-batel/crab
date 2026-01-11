#pragma once

namespace crab {

  /**
   * @brief Utility class for easily creating a Visitor instance when using
   * std::visit and alike
   */
  template<typename... Functions>
  struct cases final : Functions... {
    using Functions::operator()...;
  };

}

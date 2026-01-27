#pragma once

namespace crab::boxed {
  template<typename T>
  class Box;

  namespace impl {
    template<typename T>
    struct BoxStorage;
  }

}

#pragma once

namespace crab::result {
  template<typename T, typename E>
  class Result;

  template<typename T>
  struct Ok;

  template<typename E>
  struct Err;
}

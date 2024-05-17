// ReSharper disable CppNonExplicitConversionOperator
// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once

#include <preamble.hpp>

#include "ref.hpp"

namespace crab::rc {
  template<typename T>
  class Rc;

  namespace helper {
    template<typename T>
    class RcInterior {
      friend class Rc<T>;
      usize ref_count;
      usize weak_ref_count;
      T data;
    };
  }
}

/**
 * @brief Reference Counting for a value of type T on the heap, equivalent to std::shared_ptr but with
 * prevented interior mutability.
 */
template<typename T> requires (not std::is_array_v<T>)
class Rc final {
  using Interior = crab::rc::helper::RcInterior<T>;

  Interior *interior;

  explicit Rc(Interior *interior) : interior{interior} {
    debug_assert(interior != nullptr, "Corrupted Rc<T>: Cannot construct a NULL Rc");
  }

public:
  [[nodiscard]]
  Rc from_owned_unchecked(const T *box) {
    return Rc();
  }

  operator const T&() const { return *raw_ptr(); }

  operator const T*() const { return raw_ptr(); }

  const T *operator ->() const { return raw_ptr(); }

  operator const Ref<T>() const { return as_ref(); }

  /**
   * @brief Gets a Ref<T> object to the value inside.
   */
  [[nodiscard]] Ref<T> as_ref() const { return *raw_ptr(); }

private:
  const T *raw_ptr() const {
    debug_assert(data != nullptr, "Corrupted Rc<T>, data is nullptr");
    debug_assert(ref_count != nullptr, "Corrupted Rc<T>, ref_count is nullptr");
    return data;
  }
};

namespace crab {
  template<typename T, typename... Args> requires std::is_constructible_v<T, Args...>
  Rc<T> make_rc(Args... args) {
    return Rc<T>::from_owned_unchecked(new T{args...});
  }
}

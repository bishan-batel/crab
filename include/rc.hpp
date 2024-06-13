// ReSharper disable CppNonExplicitConversionOperator
// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once

#include <atomic>
#include <memory>
#include <preamble.hpp>

#include "ref.hpp"

namespace crab::rc {
  template<typename T>
  class Rc;

  namespace helper {
    template<bool = false>
    struct RcCounter final {
      using type = usize;
    };

    template<>
    struct RcCounter<true> final {
      using type = std::atomic<usize>;
    };

    template<typename Contained, bool thread_safe = false>
    class RcInterior final {
      friend class Rc<Contained>;
      using Counter = typename RcCounter<thread_safe>::type;

      Counter weak_ref_count;
      Counter ref_count;
      Contained *data;

      RcInterior(const usize ref_count, const usize weak_ref_count, Contained *const data)
        : weak_ref_count{weak_ref_count}, ref_count{ref_count}, data{data} {}

      auto increment_ref_count() -> void {
        debug_assert(ref_count != 0, "Corrupted Rc<T>: ref_count being increased after reaching zero");
        ++ref_count;
      }

      auto decrement_ref_count() -> void {
        debug_assert(weak_ref_count != 0, "Corrupted Rc<T>: weak_ref_count being decreased after reaching zero");
        --weak_ref_count;
      }

      auto is_unique() const -> bool {
        return ref_count == 1;
      }

      auto is_data_valid() const -> bool {
        return data != nullptr;
      }

      auto should_free_data() const -> bool {
        return ref_count == 0 and weak_ref_count == 0 and is_data_valid();
      }

      auto should_free_self() const -> bool {
        return ref_count == 0 and is_data_valid();
      }

      auto free_data() const -> void {
        debug_assert(is_data_valid(), "Invalid use of Rc<T>: Data cannot be double freed.");
        debug_assert(
          should_free_data(),
          "Invalid use of Rc<T>: Data cannot be freed when there are existing references."
        );
        delete data;
      }

      template<typename Derived=Contained> requires std::is_base_of_v<Contained, Derived>
      auto raw_ptr() const -> Derived * {
        debug_assert(data != nullptr, "Invalid access of Rc<T> or RcMut<T>, data is nullptr");
        return static_cast<Derived*>(data);
      }

      template<typename Base> requires std::is_base_of_v<Base, Contained>
      auto downcast() -> RcInterior<Base>& {
        return *reinterpret_cast<RcInterior<Base>*>(this);
      }
    };
  }
}

/**
 * @brief Reference Counting for a value of type T on the heap, equivalent to std::shared_ptr but with
 * prevented interior mutability.
 */
template<typename Contained> requires (not std::is_array_v<Contained>)
class Rc final {
  using Interior = crab::rc::helper::RcInterior<Contained>;

  Interior *interior;

  explicit Rc(Interior *interior)
    : interior{interior} {
    debug_assert(interior != nullptr, "Corrupted Rc<T>: Cannot construct a NULL Rc");
  }

  auto destruct() -> void {
    if (interior) {
      interior->decrement_ref_count();
      interior = nullptr;
    }
  }

public:
  [[nodiscard]]
  static auto from_owned_unchecked(Contained *box) -> Rc {
    return Rc{
      new Interior{
        1,
        0,
        box
      }
    };
  }

  Rc(const Rc &from)
    : interior{from.interior} {
    debug_assert(
      interior != nullptr and interior->is_data_valid(),
      "Invalid use of Rc<T>, copied from an invalidated Rc, most likely a use-after-move"
    );

    interior->increment_ref_count();
  }

  Rc(Rc &&from) noexcept
    : interior{std::exchange(from.interior, nullptr)} {
    debug_assert(
      interior != nullptr,
      "Invalid use of Rc<T>, moved from an invalidated Rc, most likely a use-after-move"
    );
  }

  ~Rc() {
    destruct();
  }

  template<typename Base> requires std::is_base_of_v<Base, Contained>
  auto downcast() -> Rc<Base> {
    Rc<Base> casted{&get_interior()->template downcast<Base>()};

    debug_assert(
      interior->is_data_valid(),
      "Invalid use of Rc<T>, copied from an invalidated Rc, most likely a use-after-move"
    );

    casted.get_interior()->increment_ref_count();

    return casted;
  }

  auto operator=(const Rc &from) -> Rc& {
    if (&from == this) return *this;

    destruct();

    interior = from.interior;

    return *this;
  }

  auto operator=(Rc &&from) noexcept -> Rc& {
    if (&from == this) return *this;

    destruct();

    interior = std::exchange(from.interior, nullptr);

    return *this;
  }

  operator const Contained&() const { return *raw_ptr(); }

  operator const Contained*() const { return raw_ptr(); }

  const Contained *operator ->() const { return raw_ptr(); }

  const Contained& operator *() const { return *raw_ptr(); }

  operator const Ref<Contained>() const { return as_ref(); }

  /**
   * @brief Gets a Ref<T> object to the value inside.
   */
  [[nodiscard]] auto as_ref() const -> Ref<Contained> { return *raw_ptr(); }

  [[nodiscard]] auto is_unique() const -> bool {
    return is_valid() and get_interior()->is_unique();
  }

  /**
   * @brief Queries if this instance has been
   */
  [[nodiscard]] auto is_valid() const -> bool {
    return interior != nullptr;
  }

  /**
  * @brief Returns a reference to the underlying data
  */
  auto get() const -> const Contained& {
    return *raw_ptr();
  }

  /**
   * @brief Returns a raw pointer to the underlying data
   */
  auto raw_ptr() const -> const Contained * {
    return get_interior()->raw_ptr();
  }

private:
  auto get_interior() const -> Interior * {
    debug_assert(
      is_valid(),
      "Invalid use of Rc<T>, Interior is nullptr - this is most likely the result of a use-after-move."
    );
    return interior;
  }
};

namespace crab {
  template<typename T, typename... Args> requires std::is_constructible_v<T, Args...>
  auto make_rc(Args... args) -> Rc<T> {
    return Rc<T>::from_owned_unchecked(new T{args...});
  }
}

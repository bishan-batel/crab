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
      using Counter = typename RcCounter<thread_safe>::type;

      Counter weak_ref_count;
      Counter ref_count;
      Contained *data;

    public:
      RcInterior(const usize ref_count, const usize weak_ref_count, Contained *const data)
        : weak_ref_count{weak_ref_count}, ref_count{ref_count}, data{data} {}

      auto increment_ref_count() -> void {
        debug_assert(ref_count != 0, "Corrupted Rc<T>: ref_count being increased after reaching zero");
        ++ref_count;
      }

      auto decrement_ref_count() -> void {
        debug_assert(ref_count != 0, "Corrupted Rc<T>: ref_count being decreased after reaching zero");
        --ref_count;
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
      auto upcast() -> RcInterior<Base> * {
        return reinterpret_cast<RcInterior<Base>*>(this);
      }

      template<typename Derived> requires std::derived_from<Derived, Contained>
      auto downcast() -> Option<RcInterior<Derived>*> {
        if (dynamic_cast<Derived*>(this->data) == nullptr) {
          return none;
        }
        return some(reinterpret_cast<RcInterior<Derived>*>(this));
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

  Raw<Interior> interior;

  explicit Rc(Interior *interior)
    : interior{interior} {
    debug_assert(interior != nullptr, "Corrupted Rc<T>: Cannot construct a NULL Rc");
  }

  auto destruct() -> void {
    if (interior) {
      interior->decrement_ref_count();

      if (interior->should_free_data()) {
        interior->free_data();
      }

      if (interior->should_free_self()) {
        delete interior;
      }
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

  [[nodiscard]]
  static auto from_rc_interior_unchecked(Interior *interior) -> Rc {
    return Rc{interior};
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

  template<typename Derived> requires std::derived_from<Derived, Contained>
  Rc(const Rc<Derived> &from)
    : interior{
      from.get_interior()->template upcast<Contained>()
    } {
    get_interior()->increment_ref_count();
  }

  template<typename Derived> requires std::derived_from<Derived, Contained>
  Rc(Rc<Derived> &&from)
    : interior{
      std::exchange(from.get_interior()->template upcast<Contained>(), nullptr)
    } {}

  ~Rc() {
    destruct();
  }

  /**
   * @brief Converts Rc<Derived> -> Rc<Base>
   */
  template<typename Base> requires std::is_base_of_v<Base, Contained>
  auto upcast() -> Rc<Base> {
    crab::rc::helper::RcInterior<Base> *i = get_interior()->template upcast<Base>();
    Rc<Base> casted = Rc<Base>::from_rc_interior_unchecked(i);

    debug_assert(
      interior->is_data_valid(),
      "Invalid use of Rc<T>, copied from an invalidated Rc, most likely a use-after-move"
    );

    casted.get_interior()->increment_ref_count();

    return casted;
  }

  /**
   * @brief Attempts to convert Rc<Base> -> Rc<Derived>
   */
  template<typename Derived> requires std::derived_from<Derived, Contained>
  auto downcast() -> Option<Rc<Derived>> {
    auto inner = get_interior()->template downcast<Derived>();

    if (inner.is_none()) {
      return crab::none;
    }

    Rc<Derived> casted = Rc<Derived>::from_rc_interior_unchecked(crab::unwrap(std::move(inner)));

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

  auto get_interior() const -> Interior * {
    debug_assert(
      is_valid(),
      "Invalid use of Rc<T>, Interior is nullptr - this is most likely the result of a use-after-move."
    );
    return interior;
  }
};

template<typename Contained> requires (not std::is_array_v<Contained>)
class RcMut final {
  using Interior = crab::rc::helper::RcInterior<Contained>;

  Raw<Interior> interior;

  explicit RcMut(Interior *interior)
    : interior{interior} {
    debug_assert(interior != nullptr, "Corrupted RcMut<T>: Cannot construct a NULL Rc");
  }

  auto destruct() -> void {
    if (interior) {
      interior->decrement_ref_count();

      if (interior->should_free_data()) {
        interior->free_data();
      }

      if (interior->should_free_self()) {
        delete interior;
      }
      interior = nullptr;
    }
  }

public:
  [[nodiscard]]
  static auto from_owned_unchecked(Contained *box) -> RcMut {
    return RcMut{
      new Interior{
        1,
        0,
        box
      }
    };
  }

  [[nodiscard]]
  static auto from_rc_interior_unchecked(Interior *interior) -> RcMut {
    return RcMut{interior};
  }

  RcMut(const RcMut &from)
    : interior{from.interior} {
    debug_assert(
      interior != nullptr and interior->is_data_valid(),
      "Invalid use of RcMut<T>, copied from an invalidated RcMut, most likely a use-after-move"
    );

    interior->increment_ref_count();
  }

  RcMut(RcMut &&from) noexcept
    : interior{std::exchange(from.interior, nullptr)} {
    debug_assert(
      interior != nullptr,
      "Invalid use of RcMut<T>, moved from an invalidated RcMut, most likely a use-after-move"
    );
  }

  template<typename Derived> requires std::derived_from<Derived, Contained>
  RcMut(const RcMut<Derived> &from)
    : interior{
      from.get_interior()->template upcast<Contained>()
    } {
    get_interior()->increment_ref_count();
  }

  template<typename Derived> requires std::derived_from<Derived, Contained>
  RcMut(RcMut<Derived> &&from)
    : interior{
      std::exchange(from.get_interior()->template upcast<Contained>(), nullptr)
    } {}

  ~RcMut() {
    destruct();
  }

  /**
   * @brief Converts RcMut<Derived> -> RcMut<Base>
   */
  template<typename Base> requires std::is_base_of_v<Base, Contained>
  auto upcast() -> RcMut<Base> {
    crab::rc::helper::RcInterior<Base> *i = get_interior()->template upcast<Base>();
    RcMut<Base> casted = RcMut<Base>::from_rc_interior_unchecked(i);

    debug_assert(
      interior->is_data_valid(),
      "Invalid use of RcMut<T>, copied from an invalidated RcMut, most likely a use-after-move"
    );

    casted.get_interior()->increment_ref_count();

    return casted;
  }

  /**
   * @brief Attempts to convert RcMut<Base> -> RcMut<Derived>
   */
  template<typename Derived> requires std::derived_from<Derived, Contained>
  auto downcast() -> Option<RcMut<Derived>> {
    auto inner = get_interior()->template downcast<Derived>();

    if (inner.is_none()) {
      return crab::none;
    }

    RcMut<Derived> casted = RcMut<Derived>::from_rc_interior_unchecked(crab::unwrap(std::move(inner)));

    debug_assert(
      interior->is_data_valid(),
      "Invalid use of RcMut<T>, copied from an invalidated RcMut, most likely a use-after-move"
    );

    casted.get_interior()->increment_ref_count();

    return casted;
  }

  auto operator=(const RcMut &from) -> RcMut& {
    if (&from == this) return *this;

    destruct();

    interior = from.interior;

    return *this;
  }

  auto operator=(RcMut &&from) noexcept -> RcMut& {
    if (&from == this) return *this;

    destruct();

    interior = std::exchange(from.interior, nullptr);

    return *this;
  }

  /**
   * Converts to an immutable Rc<T>
   */
  operator Rc<Contained>() const {
    auto rc = Rc<Contained>::from_rc_interior_unchecked(get_interior());
    get_interior()->increment_ref_count();
    return rc;
  }

  /**
   * Converts to an immutable Rc<T>
   */
  template<typename Base> requires std::is_base_of_v<Base, Contained>
  operator Rc<Base>() const {
    return upcast<Base>();
  }

  operator Contained&() const { return *raw_ptr(); }

  operator Contained*() const { return raw_ptr(); }

  Contained *operator ->() const { return raw_ptr(); }

  Contained& operator *() const { return *raw_ptr(); }

  operator RefMut<Contained>() const { return as_ref(); }

  /**
   * @brief Gets a Ref<T> object to the value inside.
   */
  [[nodiscard]] auto as_ref() const -> RefMut<Contained> { return *raw_ptr(); }

  /**
   * @brief Queries if this is the only instance of RcMut<T> (or Rc<T>) that remains.
   * This will also consider Weak references.
   */
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
  auto get() const -> Contained& {
    return *raw_ptr();
  }

  /**
   * @brief Returns a raw pointer to the underlying data
   */
  auto raw_ptr() const -> Contained * {
    return get_interior()->raw_ptr();
  }

  auto get_interior() const -> Interior * {
    debug_assert(
      is_valid(),
      "Invalid use of RcMut<T>, Interior is nullptr - this is most likely the result of a use-after-move."
    );
    return interior;
  }
};

namespace crab {
  template<typename T, typename... Args> requires std::is_constructible_v<T, Args...>
  auto make_rc(Args... args) -> Rc<T> {
    return Rc<T>::from_owned_unchecked(new T{args...});
  }

  template<typename T, typename... Args> requires std::is_constructible_v<T, Args...>
  auto make_rc_mut(Args... args) -> Rc<T> {
    return RcMut<T>::from_owned_unchecked(new T{args...});
  }
}

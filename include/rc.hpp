#pragma once

#include <atomic>
#include <bit>
#include <preamble.hpp>

#include "box.hpp"
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
      Contained* data;

    public:

      RcInterior(
        const usize ref_count,
        const usize weak_ref_count,
        Contained* const data
      ):
          weak_ref_count{weak_ref_count}, ref_count{ref_count}, data{data} {}

      auto increment_ref_count() -> void {
        debug_assert(
          ref_count != 0,
          "Corrupted Rc<T>: ref_count being increased after reaching zero"
        );
        ++ref_count;
      }

      auto decrement_ref_count() -> void {
        debug_assert(
          ref_count != 0,
          "Corrupted Rc<T>: ref_count being decreased after reaching zero"
        );
        --ref_count;
      }

      [[nodiscard]] auto is_unique() const -> bool { return ref_count == 1; }

      [[nodiscard]] auto get_ref_count() const -> usize { return ref_count; }

      [[nodiscard]] auto is_data_valid() const -> bool {
        return data != nullptr;
      }

      [[nodiscard]] auto should_free_data() const -> bool {
        return ref_count == 0 and weak_ref_count == 0 and is_data_valid();
      }

      [[nodiscard]] auto should_free_self() const -> bool {
        return ref_count == 0 and is_data_valid();
      }

      auto free_data() const -> void {
        debug_assert(
          is_data_valid(),
          "Invalid use of Rc<T>: Data cannot be double freed."
        );
        debug_assert(
          should_free_data(),
          "Invalid use of Rc<T>: Data cannot be freed when there are existing "
          "references."
        );
        delete data;
      }

      template<typename Derived = Contained>
      requires std::is_base_of_v<Contained, Derived>
      [[nodiscard]] auto raw_ptr() const -> Derived* {
        debug_assert(
          data != nullptr,
          "Invalid access of Rc<T> or RcMut<T>, data is nullptr"
        );
        return static_cast<Derived*>(data);
      }

      template<typename Base>
      requires std::derived_from<Contained, Base>
      [[nodiscard]] auto upcast() const -> RcInterior<Base>* {
        return std::bit_cast<RcInterior<Base>*>(this);
      }

      template<std::derived_from<Contained> Derived>
      [[nodiscard]] auto downcast() const -> Option<RcInterior<Derived>*> {
        if (dynamic_cast<Derived*>(this->data) == nullptr) {
          return none;
        }
        return some(std::bit_cast<RcInterior<Derived>*>(this));
      }
    };
  } // namespace helper
} // namespace crab::rc

/**
 * @brief Reference Counting for a value of type T on the heap, equivalent to
 * std::shared_ptr but with prevented interior mutability.
 */
template<typename Contained>
class Rc final {
  using Interior = crab::rc::helper::RcInterior<Contained>;

  Interior* interior;

  /**
   * Private unsafe constructor wrapping a raw interior
   *
   * Public version is from_interior_unchecked
   */
  explicit Rc(Interior* interior): interior{interior} {
    debug_assert(
      interior != nullptr,
      "Corrupted Rc<T>: Cannot construct a NULL Rc"
    );
  }

  /**
   * Destructor operations for this class
   */
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

  /**
   * @brief Wraps a heap allocatated (with ::operator new), this
   * function is UB if the given pointer is being managed by any other RAII
   * wrapper
   */
  [[nodiscard]]
  static auto from_owned_unchecked(Contained* box) -> Rc {
    return Rc{
      new Interior{1, 0, box}
    };
  }

  /**
   * @brief Wraps a RcInterior<T>, this
   * function is UB if the given pointer is being managed by any other RAII
   * wrapper or is violating strict aliasing (most likely you want to use
   * from_owned_unchecked)
   */
  [[nodiscard]]
  static auto from_rc_interior_unchecked(Interior* interior) -> Rc {
    return Rc{interior};
  }

  /**
   * Copy constructor
   */
  Rc(const Rc& from): interior{from.interior} {
    debug_assert(
      interior != nullptr and interior->is_data_valid(),
      "Invalid use of Rc<T>, copied from an invalidated Rc, most likely a "
      "use-after-move"
    );

    interior->increment_ref_count();
  }

  /**
   * Move constructor
   */
  Rc(Rc&& from) noexcept: interior{std::exchange(from.interior, nullptr)} {
    debug_assert(
      interior != nullptr,
      "Invalid use of Rc<T>, moved from an invalidated Rc, most likely a "
      "use-after-move"
    );
  }

  /**
   * @brief Copy constructor from a RcMut<Derived> -> RcMut<Base>
   */
  template<std::derived_from<Contained> Derived>
  Rc(const Rc<Derived>& from): // NOLINT(*-explicit-constructor)
      interior{from.get_interior()->template upcast<Contained>()} {
    get_interior()->increment_ref_count();
  }

  /**
   * @brief Move constructor from a RcMut<Derived> -> RcMut<Base>
   */
  template<std::derived_from<Contained> Derived>
  Rc(Rc<Derived>&& from): // NOLINT(*-explicit-constructor)
      interior{std::exchange(from.get_interior(), nullptr)
                 ->template upcast<Contained>()} {}

  /**
   * @brief Converts an singlely-owned box into a shared pointer
   */
  Rc(Box<Contained> from): // NOLINT(*-explicit-constructor)
      interior{
        new Interior{1, 0, Box<Contained>::unwrap(std::move(from))}
  } {}

  /**
   * @brief Converts an singlely-owned box into a shared pointer upcast
   */
  template<std::derived_from<Contained> Derived>
  Rc(Box<Derived> from): // NOLINT(*-explicit-constructor)
      Rc{Box<Contained>{std::forward<Box<Derived>>(from)}} {}

  /**
   * @brief Destructor
   */
  ~Rc() { destruct(); }

  /**
   * @brief Converts Rc<Derived> -> Rc<Base>
   */
  template<typename Base>
  auto upcast() const -> Rc<Base> {
    static_assert(
      std::derived_from<Contained, Base>,
      "Cannot upcast to a Base that Contained does not derive from"
    );

    crab::rc::helper::RcInterior<Base>* i =
      get_interior()->template upcast<Base>();
    Rc<Base> casted = Rc<Base>::from_rc_interior_unchecked(i);

    debug_assert(
      interior->is_data_valid(),
      "Invalid use of Rc<T>, copied from an invalidated Rc, most likely a "
      "use-after-move"
    );

    casted.get_interior()->increment_ref_count();

    return casted;
  }

  /**
   * @brief Attempts to convert Rc<Base> -> Rc<Derived>
   */
  template<std::derived_from<Contained> Derived>
  auto downcast() const -> Option<Rc<Derived>> {
    auto inner = get_interior()->template downcast<Derived>();

    if (inner.is_none()) {
      return crab::none;
    }

    Rc<Derived> casted =
      Rc<Derived>::from_rc_interior_unchecked(std::move(inner).unwrap());

    debug_assert(
      interior->is_data_valid(),
      "Invalid use of Rc<T>, copied from an invalidated Rc, most likely a "
      "use-after-move"
    );

    casted.get_interior()->increment_ref_count();

    return casted;
  }

  /**
   * Copy assignment
   */
  auto operator=(const Rc& from) -> Rc& {
    if (&from == this) {
      return *this;
    }

    destruct();

    interior = from.interior;

    return *this;
  }

  /**
   * Move assignment
   */
  auto operator=(Rc&& from) noexcept -> Rc& {
    if (&from == this) {
      return *this;
    }

    destruct();

    interior = std::exchange(from.interior, nullptr);

    return *this;
  }

  /**
   * Implicitly coerce into a reference to the contained
   */
  operator const Contained&() const { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  /**
   * Implicitly coerce into a reference to a pointer to the contained value
   */
  operator const Contained*() const { // NOLINT(*-explicit-constructor)
    return raw_ptr();
  }

  /**
   * Implicitly coerce into a reference to the contained
   */
  operator Ref<Contained>() const { // NOLINT(*-explicit-constructor)
    return as_ref();
  }

  [[nodiscard]] auto operator->() const -> const Contained* {
    return raw_ptr();
  }

  [[nodiscard]] auto operator*() const -> const Contained& {
    return *raw_ptr();
  }

  /**
   * @brief Gets a Ref<T> object to the value inside.
   */
  [[nodiscard]] auto as_ref() const -> Ref<Contained> { return *raw_ptr(); }

  /**
   * @brief Queries if this is the only instance of RcMut<T> (or Rc<T>) that
   * remains. This will also consider Weak references.
   */
  [[nodiscard]] auto is_unique() const -> bool {
    return is_valid() and get_interior()->is_unique();
  }

  /**
   * @brief How many references exist to the given resource
   */
  [[nodiscard]] auto get_ref_count() const -> usize {
    if (not is_valid()) {
      return 0;
    }
    return get_interior()->get_ref_count();
  }

  /**
   * @brief Queries if this instance has been
   */
  [[nodiscard]] auto is_valid() const -> bool { return interior != nullptr; }

  /**
   * @brief Returns a reference to the underlying data
   */
  [[nodiscard]] auto get() const -> const Contained& { return *raw_ptr(); }

  /**
   * @brief Returns a raw pointer to the underlying data
   */
  [[nodiscard]] auto raw_ptr() const -> const Contained* {
    return get_interior()->raw_ptr();
  }

  /**
   * @brief Gets raw pointer to the RcInterior, do not use this unless you have
   * a very good reason for messing with the invariants of Rc
   */
  auto get_interior() const -> Interior* {
    debug_assert(
      is_valid(),
      "Invalid use of Rc<T>, Interior is nullptr - this is most likely the "
      "result of a use-after-move."
    );
    return interior;
  }

  /**
   * @brief UNSAFE: Gets raw pointer to the RcInterior, do not use this unless
   * you have a reason to fuck with Rc directly
   */
  auto get_interior() -> Interior*& {
    debug_assert(
      is_valid(),
      "Invalid use of Rc<T>, Interior is nullptr - this is most likely the "
      "result of a use-after-move."
    );
    return interior;
  }
};

template<typename Contained>
class RcMut final {
  using Interior = crab::rc::helper::RcInterior<Contained>;

  Interior* interior;

  /**
   * Private unsafe constructor wrapping a raw interior
   *
   * Public version is from_interior_unchecked
   */
  explicit RcMut(Interior* interior): interior{interior} {
    debug_assert(
      interior != nullptr,
      "Corrupted RcMut<T>: Cannot construct a NULL Rc"
    );
  }

  /**
   * Destructor operations for this class
   */
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

  /**
   * @brief Wraps a heap allocatated (with ::operator new), this
   * function is UB if the given pointer is being managed by any other RAII
   * wrapper
   */
  [[nodiscard]]
  static auto from_owned_unchecked(Contained* box) -> RcMut {
    return RcMut{
      new Interior{1, 0, box}
    };
  }

  /**
   * @brief Wraps a RcInterior<T>, this
   * function is UB if the given pointer is being managed by any other RAII
   * wrapper or is violating strict aliasing (most likely you want to use
   * from_owned_unchecked)
   */
  [[nodiscard]]
  static auto from_rc_interior_unchecked(Interior* interior) -> RcMut {
    return RcMut{interior};
  }

  /**
   * Copy constructor
   */
  RcMut(const RcMut& from): interior{from.interior} {
    debug_assert(
      interior != nullptr and interior->is_data_valid(),
      "Invalid use of RcMut<T>, copied from an invalidated RcMut, most likely "
      "a use-after-move"
    );

    interior->increment_ref_count();
  }

  /**
   * Move constructor
   */
  RcMut(RcMut&& from) noexcept:
      interior{std::exchange(from.interior, nullptr)} {
    debug_assert(
      interior != nullptr,
      "Invalid use of RcMut<T>, moved from an invalidated RcMut, most likely a "
      "use-after-move"
    );
  }

  /**
   * @brief Copy constructor from a RcMut<Derived> -> RcMut<Base>
   */
  template<std::derived_from<Contained> Derived>
  RcMut(const RcMut<Derived>& from): // NOLINT(*-explicit-constructor)
      interior{from.get_interior()->template upcast<Contained>()} {
    get_interior()->increment_ref_count();
  }

  /**
   * @brief Move constructor from a RcMut<Derived> -> RcMut<Base>
   */
  template<std::derived_from<Contained> Derived>
  RcMut(RcMut<Derived>&& from): // NOLINT(*-explicit-constructor)
      interior{std::exchange(from.get_interior(), nullptr)
                 ->template upcast<Contained>()} {}

  /**
   * @brief Converts an singlely-owned box into a shared pointer
   */
  RcMut(Box<Contained> from): // NOLINT(*-explicit-constructor)
      interior{
        new Interior{1, 0, Box<Contained>::unwrap(std::move(from))}
  } {}

  /**
   * @brief Converts an singlely-owned box into a shared pointer upcast
   */
  template<std::derived_from<Contained> Derived>
  RcMut(Box<Derived> from): // NOLINT(*-explicit-constructor)
      RcMut<Contained>{Box<Contained>{std::move(from)}} {}

  /**
   * @brief Destructor
   */
  ~RcMut() { destruct(); }

  /**
   * @brief Converts RcMut<Derived> -> RcMut<Base>
   */
  template<class Base>
  auto upcast() const -> RcMut<Base> {
    static_assert(
      std::derived_from<Contained, Base>,
      "Cannot upcast to a Base that Contained does not derive from"
    );

    crab::rc::helper::RcInterior<Base>* i =
      get_interior()->template upcast<Base>();
    RcMut<Base> casted = RcMut<Base>::from_rc_interior_unchecked(i);

    debug_assert(
      interior->is_data_valid(),
      "Invalid use of RcMut<T>, copied from an invalidated RcMut, most likely "
      "a use-after-move"
    );

    casted.get_interior()->increment_ref_count();

    return casted;
  }

  /**
   * @brief Attempts to convert RcMut<Base> -> RcMut<Derived>
   */
  template<std::derived_from<Contained> Derived>
  auto downcast() const -> Option<RcMut<Derived>> {
    auto inner = get_interior()->template downcast<Derived>();

    if (inner.is_none()) {
      return crab::none;
    }

    RcMut<Derived> casted = RcMut<Derived>::from_rc_interior_unchecked( //
      std::move(inner).unwrap()
    );

    debug_assert(
      interior->is_data_valid(),
      "Invalid use of RcMut<T>, copied from an invalidated RcMut, most likely "
      "a use-after-move"
    );

    casted.get_interior()->increment_ref_count();

    return casted;
  }

  /**
   * Copy assignment
   */
  auto operator=(const RcMut& from) -> RcMut& {
    if (&from == this) {
      return *this;
    }

    destruct();

    interior = from.interior;

    return *this;
  }

  /**
   * Move assignment
   */
  auto operator=(RcMut&& from) noexcept -> RcMut& {
    if (&from == this) {
      return *this;
    }

    destruct();

    interior = std::exchange(from.interior, nullptr);

    return *this;
  }

  /**
   * Converts to an immutable Rc<T>
   */
  operator Rc<Contained>() const { // NOLINT(*-explicit-constructor)
    auto rc = Rc<Contained>::from_rc_interior_unchecked(get_interior());
    get_interior()->increment_ref_count();
    return rc;
  }

  /**
   * Converts to an immutable Rc<T>
   */
  template<typename Base>
  requires std::derived_from<Contained, Base>
  operator Rc<Base>() const { // NOLINT(*-explicit-constructor)
    return upcast<Base>();
  }

  /**
   * Implicitly coerce into a reference to the contained
   */
  operator Contained&() const { // NOLINT(*-explicit-constructor)
    return *raw_ptr();
  }

  /**
   * Implicitly coerce into a reference to a pointer to the contained value
   */
  operator Contained*() const { // NOLINT(*-explicit-constructor)
    return raw_ptr();
  }

  /**
   * Implicitly coerce into a reference to the contained
   */
  operator RefMut<Contained>() const { // NOLINT(*-explicit-constructor)
    return as_ref();
  }

  [[nodiscard]] auto operator->() const -> Contained* { return raw_ptr(); }

  [[nodiscard]] auto operator*() const -> Contained& { return *raw_ptr(); }

  [[nodiscard]] auto operator->() -> Contained* { return raw_ptr(); }

  [[nodiscard]] auto operator*() -> Contained& { return *raw_ptr(); }

  /**
   * @brief Gets a Ref<T> object to the value inside.
   */
  [[nodiscard]] auto as_ref() const -> RefMut<Contained> { return *raw_ptr(); }

  /**
   * @brief Queries if this is the only instance of RcMut<T> (or Rc<T>) that
   * remains. This will also consider Weak references.
   */
  [[nodiscard]] auto is_unique() const -> bool {
    return is_valid() and get_interior()->is_unique();
  }

  /**
   * @brief How many references exist to the given resource
   */
  [[nodiscard]] auto get_ref_count() const -> usize {
    if (not is_valid()) {
      return 0;
    }
    return get_interior()->get_ref_count();
  }

  /**
   * @brief Queries if this instance has been
   */
  [[nodiscard]] auto is_valid() const -> bool { return interior != nullptr; }

  /**
   * @brief Returns a reference to the underlying data
   */
  [[nodiscard]] auto get() const -> Contained& { return *raw_ptr(); }

  /**
   * @brief Returns a raw pointer to the underlying data
   */
  [[nodiscard]] auto raw_ptr() const -> Contained* {
    return get_interior()->raw_ptr();
  }

  /**
   * @brief Gets raw pointer to the RcInterior, do not use this unless you have
   * a reason to fuck with Rc directly
   */
  auto get_interior() const -> Interior* {
    debug_assert(
      is_valid(),
      "Invalid use of RcMut<T>, Interior is nullptr - this is most likely the "
      "result of a use-after-move."
    );
    return interior;
  }

  /**
   * @brief Gets raw pointer to the RcInterior, do not use this unless you have
   * a very good reason for messing with the invariants of Rc
   */
  auto get_interior() -> Interior*& {
    debug_assert(
      is_valid(),
      "Invalid use of RcMut<T>, Interior is nullptr - this is most likely the "
      "result of a use-after-move."
    );
    return interior;
  }
};

namespace crab {
  /**
   * Creates a new reference counted instance of T
   * @tparam T The type to be heap allocated & reference counted
   * @tparam Args Argument types to be passed to T's constructor
   * @param args Arguments to be passed to T's constructor
   */
  template<typename T, typename... Args>
  requires std::constructible_from<T, Args...>
  auto make_rc(Args&&... args) -> Rc<T> {
    return Rc<T>::from_owned_unchecked(new T{std::forward<Args>(args)...});
  }

  /**
   * Creates a new mutable reference counted instance of 'const T'
   * @tparam T The type to be heap allocated & reference counted
   * @tparam Args Argument types to be passed to T's constructor
   * @param args Arguments to be passed to T's constructor
   */
  template<typename T, typename... Args>
  requires std::constructible_from<T, Args...>
  auto make_rc_mut(Args&&... args) -> RcMut<T> {
    return RcMut<T>::from_owned_unchecked(new T{std::forward<Args>(args)...});
  }
} // namespace crab

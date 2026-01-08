#pragma once

#include <bit>
#include <atomic>

#include <crab/preamble.hpp>
#include <crab/box.hpp>
#include <crab/ref.hpp>
#include "crab/core.hpp"
#include "crab/mem/take.hpp"

namespace crab {
  namespace rc {
    template<typename T>
    class Rc;

    template<typename T>
    class RcMut;
  }

  namespace opt {
    template<typename T, template<typename Inner> typename Container>
    struct RcStorage;

    template<typename T>
    struct Storage;

    template<typename T>
    struct Storage<::crab::rc::Rc<T>> final {
      using type = RcStorage<T, ::crab::rc::Rc>;
    };

    template<typename T>
    struct Storage<::crab::rc::RcMut<T>> final {
      using type = RcStorage<T, ::crab::rc::RcMut>;
    };

  }

  namespace rc {
    namespace impl {
      template<typename T, bool thread_safe = false>
      class RcInterior final {
        using Counter = std::conditional_t<thread_safe, std::atomic_size_t, usize>;

        Counter weak_ref_count;
        Counter ref_count;
        T* data;

      public:

        RcInterior(const usize ref_count, const usize weak_ref_count, T* const data):
            weak_ref_count{weak_ref_count}, ref_count{ref_count}, data{data} {}

        CRAB_INLINE_CONSTEXPR auto increment_ref_count(const SourceLocation loc = SourceLocation::current()) -> void {
          debug_assert_transparent(
            not should_free_data(),
            loc,
            "Corrupted Rc<T>: ref_count being increased after reaching zero"
          );
          ++ref_count;
        }

        CRAB_INLINE_CONSTEXPR auto decrement_ref_count(const SourceLocation loc = SourceLocation::current()) -> void {
          debug_assert_transparent(
            not should_free_data(),
            loc,
            "Corrupted Rc<T>: ref_count being decreased after reaching zero"
          );
          --ref_count;
        }

        CRAB_NODISCARD_INLINE_CONSTEXPR auto is_unique() const -> bool {
          return ref_count == 1;
        }

        CRAB_NODISCARD_INLINE_CONSTEXPR auto get_ref_count() const -> usize {
          return ref_count;
        }

        CRAB_NODISCARD_INLINE_CONSTEXPR auto is_data_valid() const -> bool {
          return data != nullptr;
        }

        CRAB_NODISCARD_INLINE_CONSTEXPR auto should_free_data() const -> bool {
          return ref_count == 0 and is_data_valid();
        }

        CRAB_NODISCARD_INLINE_CONSTEXPR auto should_free_self() const -> bool {
          return ref_count == 0 and weak_ref_count == 0;
        }

        constexpr auto free_data(const SourceLocation loc = SourceLocation::current()) -> void {
          debug_assert_transparent(
            should_free_data(),
            loc,
            "Invalid use of Rc<T>: Data cannot be freed when there are existing "
            "references."
          );

          delete mem::take(data);
        }

        template<std::derived_from<T> Derived = T>
        CRAB_NODISCARD_INLINE_CONSTEXPR auto raw_ptr(const SourceLocation loc = SourceLocation::current()) const
          -> Derived* {
          debug_assert_transparent(is_data_valid(), loc, "Invalid access of Rc<T> or RcMut<T>, data is nullptr");
          return static_cast<Derived*>(data);
        }

        template<typename Base>
        requires std::derived_from<T, Base>
        CRAB_NODISCARD_INLINE_CONSTEXPR auto upcast() const -> RcInterior<Base>* {
          return std::bit_cast<RcInterior<Base>*>(this);
        }

        template<std::derived_from<T> Derived>
        CRAB_NODISCARD_CONSTEXPR auto downcast() const -> Option<RcInterior<Derived>*> {
          if (dynamic_cast<Derived*>(data)) {
            return some(std::bit_cast<RcInterior<Derived>*>(this));
          }

          return crab::none;
        }

        CRAB_NODISCARD auto release(const SourceLocation loc = SourceLocation::current()) -> Box<T> {
          decrement_ref_count(loc);
          return Box<T>::wrap_unchecked(std::exchange(data, nullptr));
        }
      };
    } // namespace crab::rc::helper

    /**
     * @brief Reference Counting for a value of type T on the heap, equivalent to
     * std::shared_ptr but with prevented interior mutability.
     */
    template<typename T>
    class Rc final {
      friend struct ::crab::opt::RcStorage<T, ::crab::rc::Rc>;

      using Interior = impl::RcInterior<T>;

      Interior* interior;

      /**
       * Private unsafe constructor wrapping a raw interior
       *
       * Public version is from_interior_unchecked
       */
      explicit Rc(Interior* interior, const SourceLocation loc = SourceLocation::current()): interior{interior} {
        debug_assert_transparent(interior != nullptr, loc, "Corrupted Rc<T>: Cannot construct a NULL Rc");
      }

      /**
       * Destructor operations for this class
       */
      auto destruct(const SourceLocation loc = SourceLocation::current()) -> void {
        if (interior == nullptr) {
          return;
        }

        interior->decrement_ref_count(loc);

        if (interior->should_free_data()) {
          interior->free_data(loc);
        }

        if (interior->should_free_self()) {
          delete interior;
        }
        interior = nullptr;
      }

    public:

      /**
       * @brief Wraps a heap allocatated (with ::operator new), this
       * function is UB if the given pointer is being managed by any other RAII
       * wrapper
       */
      [[nodiscard]]
      static auto from_owned_unchecked(T* box) -> Rc {
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
      Rc(const Rc& from, const SourceLocation loc = SourceLocation::current()): interior{from.interior} {

        debug_assert_transparent(
          is_valid() and interior->is_data_valid(),
          loc,
          "Invalid use of Rc<T>, copied from an invalidated Rc, most likely a "
          "use-after-move"
        );

        interior->increment_ref_count(loc);
      }

      /**
       * Move constructor
       */
      Rc(Rc&& from, const SourceLocation loc = SourceLocation::current()) noexcept:
          interior{std::exchange(from.interior, nullptr)} {
        debug_assert_transparent(
          is_valid(),
          loc,
          "Invalid use of Rc<T>, moved from an invalidated Rc, most likely a "
          "use-after-move"
        );
      }

      /**
       * @brief Copy constructor from a RcMut<Derived> -> RcMut<Base>
       */
      template<std::derived_from<T> Derived>
  Rc( // NOLINT(*-explicit-constructor)
    const Rc<Derived>& from,
    const SourceLocation loc = SourceLocation::current()
  ):
      interior{from.interior->template upcast<T>()} {
        get_interior(loc)->increment_ref_count(loc);
      }

      /**
       * @brief Move constructor from a RcMut<Derived> -> RcMut<Base>
       */
      template<std::derived_from<T> Derived>
      Rc(
        Rc<Derived>&& from,
        const SourceLocation loc = SourceLocation::current()
      ): // NOLINT(*-explicit-constructor)
          interior{
            std::exchange(from.get_interior(loc), nullptr)->template upcast<T>(),
          } {}

      /**
       * @brief Converts an singlely-owned box into a shared pointer
       */
      Rc(Box<T> from): // NOLINT(*-explicit-constructor)
          interior{
            new Interior{1, 0, Box<T>::unwrap(std::move(from))}
      } {}

      /**
       * @brief Converts an singlely-owned box into a shared pointer upcast
       */
      template<std::derived_from<T> Derived>
      Rc(Box<Derived> from): // NOLINT(*-explicit-constructor)
          Rc{Box<T>{std::forward<Box<Derived>>(from)}} {}

      /**
       * @brief Destructor
       */
      ~Rc() {
        destruct();
      }

      /**
       * @brief Converts Rc<Derived> -> Rc<Base>
       */
      template<typename Base>
      auto upcast(const SourceLocation loc = SourceLocation::current()) const -> Rc<Base> {
        static_assert(std::derived_from<T, Base>, "Cannot upcast to a Base that Contained does not derive from");

        impl::RcInterior<Base>* i = get_interior(loc)->template upcast<Base>();

        Rc<Base> casted = Rc<Base>::from_rc_interior_unchecked(i);

        debug_assert_transparent(
          interior->is_data_valid(),
          loc,
          "Invalid use of Rc<T>, copied from an invalidated Rc, most likely a "
          "use-after-move"
        );

        casted.get_interior(loc)->increment_ref_count();

        return casted;
      }

      /**
       * @brief Attempts to convert Rc<Base> -> Rc<Derived>
       */
      template<std::derived_from<T> Derived>
      auto downcast(const SourceLocation loc = SourceLocation::current()) const -> Option<Rc<Derived>> {
        return get_interior(loc)->template downcast<Derived>().map([loc](impl::RcInterior<Derived>* interior) {
          interior->increment_ref_count(loc);
          return Rc<Derived>::from_rc_interior_unchecked(interior);
        });
      }

      /**
       * Copy assignment
       */
      auto operator=(const Rc& from) -> Rc& {
        if (&from == this or from.interior == interior) {
          return *this;
        }

        destruct();

        interior = from.interior;
        interior->increment_ref_count();

        return *this;
      }

      /**
       * Move assignment
       */
      auto operator=(Rc&& from) noexcept -> Rc& {
        if (&from == this or from.interior == interior) {
          return *this;
        }

        destruct();

        interior = std::exchange(from.interior, nullptr);

        return *this;
      }

      /**
       * Implicitly coerce into a reference to the contained
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR operator const T&() const { // NOLINT(*-explicit-constructor)
        return *raw_ptr();
      }

      /**
       * Implicitly coerce into a reference to a pointer to the contained value
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR operator const T*() const { // NOLINT(*-explicit-constructor)
        return raw_ptr();
      }

      /**
       * Implicitly coerce into a reference to the contained
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR operator Ref<T>() const { // NOLINT(*-explicit-constructor)
        return as_ref();
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto operator->() const -> const T* {
        return raw_ptr();
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto operator*() const -> const T& {
        return *raw_ptr();
      }

      /**
       * @brief Gets a const reference object to the value inside.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto as_ref() const -> const T& {
        return *raw_ptr();
      }

      /**
       * @brief Queries if this is the only instance of RcMut<T> (or Rc<T>) that
       * remains. This will also consider Weak references.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto is_unique() const -> bool {
        return is_valid() and get_interior()->is_unique();
      }

      /**
       * @brief How many references exist to the given resource
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto get_ref_count() const -> usize {
        return not is_valid() ? 0 : get_interior()->get_ref_count();
      }

      /**
       * @brief Queries if this instance has been
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto is_valid() const -> bool {
        return interior != nullptr;
      }

      /**
       * @brief Returns a reference to the underlying data
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto get(const SourceLocation loc = SourceLocation::current()) const -> const T& {
        return *raw_ptr(loc);
      }

      /**
       * @brief Returns a raw pointer to the underlying data
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto raw_ptr(const SourceLocation loc = SourceLocation::current()) const
        -> const T* {
        return get_interior(loc)->raw_ptr();
      }

      /**
       * @brief Gets raw pointer to the RcInterior, do not use this unless you have
       * a very good reason for messing with the invariants of Rc
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto get_interior(const SourceLocation loc = SourceLocation::current()) const
        -> Interior* {
        debug_assert_transparent(
          is_valid(),
          loc,
          "Invalid use of Rc<T>, Interior is nullptr - this is most likely the "
          "result of a use-after-move."
        );
        return interior;
      }

      /**
       * @brief UNSAFE: Gets raw pointer to the RcInterior, do not use this unless
       * you have a reason to mess with Rc directly
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto get_interior(const SourceLocation loc = SourceLocation::current())
        -> Interior*& {
        debug_assert_transparent(
          is_valid(),
          loc,
          "Invalid use of Rc<T>, Interior is nullptr - this is most likely the "
          "result of a use-after-move."
        );
        return interior;
      }

      /**
       * @brief  Attempts to take ownership of the shared value, this will only
       * suceed if this is the only reference to the instance.
       */
      CRAB_NODISCARD_CONSTEXPR auto try_release(
        const SourceLocation loc = SourceLocation::current()
      ) && -> Option<Box<T>> {
        return std::exchange(interior, nullptr)->release(loc);
      }
    };

    template<typename T>
    class RcMut final {
      friend struct crab::opt::RcStorage<T, ::crab::rc::RcMut>;
      using Interior = impl::RcInterior<T>;

      Interior* interior;

      /**
       * Private unsafe constructor wrapping a raw interior
       *
       * Public version is from_interior_unchecked
       */
      explicit RcMut(Interior* interior, const SourceLocation loc = SourceLocation::current()): interior{interior} {
        debug_assert_transparent(interior != nullptr, loc, "Corrupted RcMut<T>: Cannot construct a NULL Rc");
      }

      /**
       * Destructor operations for this class
       */
      auto destruct(const SourceLocation loc = SourceLocation::current()) -> void {
        if (interior) {
          interior->decrement_ref_count(loc);

          if (interior->should_free_data()) {
            interior->free_data(loc);
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
      static auto from_owned_unchecked(T* box) -> RcMut {
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
      RcMut(const RcMut& from, const SourceLocation loc = SourceLocation::current()): interior{from.interior} {
        debug_assert_transparent(
          interior != nullptr and interior->is_data_valid(),
          loc,
          "Invalid use of RcMut<T>, copied from an invalidated RcMut, most likely "
          "a use-after-move"
        );

        interior->increment_ref_count(loc);
      }

      /**
       * Move constructor
       */
      RcMut(RcMut&& from, const SourceLocation loc = SourceLocation::current()) noexcept:
          interior{std::exchange(from.interior, nullptr)} {
        debug_assert_transparent(
          interior != nullptr,
          loc,
          "Invalid use of RcMut<T>, moved from an invalidated RcMut, most likely a "
          "use-after-move"
        );
      }

      /**
       * @brief Copy constructor from a RcMut<Derived> -> RcMut<Base>
       */
      template<std::derived_from<T> Derived>
  RcMut( // NOLINT(*-explicit-constructor)
    const RcMut<Derived>& from,
    const SourceLocation loc = SourceLocation::current()
  ):
      interior{from.get_interior(loc)->template upcast<T>()} {
        get_interior(loc)->increment_ref_count(loc);
      }

      /**
       * @brief Move constructor from a RcMut<Derived> -> RcMut<Base>
       */
      template<std::derived_from<T> Derived>
      RcMut(// NOLINT(*-explicit-constructor)
        RcMut<Derived>&& from,
        const SourceLocation loc = SourceLocation::current()
      ): 
          interior{std::exchange(from.get_interior(loc), nullptr)->template upcast<T>()} {}

      /**
       * @brief Converts an singlely-owned box into a shared pointer
       */
      RcMut(
        Box<T> from,
        const SourceLocation loc = SourceLocation::current()
      ): // NOLINT(*-explicit-constructor)
          interior{
            new Interior{1, 0, Box<T>::unwrap(std::move(from), loc)}
      } {}

      /**
       * @brief Converts an singlely-owned box into a shared pointer upcast
       */
      template<std::derived_from<T> Derived>
      RcMut(
        Box<Derived> from,
        const SourceLocation loc = SourceLocation::current()
      ): // NOLINT(*-explicit-constructor)
          RcMut<T>{Box<T>{std::move(from)}, loc} {}

      /**
       * @brief Destructor
       */
      ~RcMut() {
        destruct();
      }

      /**
       * @brief Converts RcMut<Derived> -> RcMut<Base>
       */
      template<class Base>
      requires std::derived_from<T, Base>
      auto upcast(const SourceLocation loc = SourceLocation::current()) const -> RcMut<Base> {
        impl::RcInterior<Base>* i{
          get_interior(loc)->template upcast<Base>(),
        };

        RcMut<Base> casted = RcMut<Base>::from_rc_interior_unchecked(i);

        debug_assert(
          interior->is_data_valid(),
          "Invalid use of RcMut<T>, copied from an invalidated RcMut, most likely "
          "a use-after-move"
        );

        casted.get_interior(loc)->increment_ref_count(loc);

        return casted;
      }

      /**
       * @brief Attempts to convert RcMut<Base> -> RcMut<Derived>
       */
      template<std::derived_from<T> Derived>
      auto downcast(const SourceLocation loc = SourceLocation::current()) const -> Option<RcMut<Derived>> {
        return get_interior(loc)->template downcast<Derived>().map([loc](impl::RcInterior<Derived>* interior) {
          interior->increment_ref_count(loc);
          return RcMut<Derived>::from_rc_interior_unchecked(interior);
        });
      }

      /**
       * Copy assignment
       */
      auto operator=(const RcMut& from) -> RcMut& {
        if (&from == this or from.interior == interior) {
          return *this;
        }

        if (from.interior == interior) {
          return *this;
        }

        destruct();

        interior = from.interior;
        interior->increment_ref_count();

        return *this;
      }

      /**
       * Move assignment
       */
      auto operator=(RcMut&& from) noexcept -> RcMut& {
        if (&from == this or from.interior == interior) {
          return *this;
        }

        destruct();

        interior = std::exchange(from.interior, nullptr);

        return *this;
      }

      /**
       * Converts to an immutable Rc<T>
       */
      operator Rc<T>() const { // NOLINT(*-explicit-constructor)
        auto rc = Rc<T>::from_rc_interior_unchecked(get_interior());
        get_interior()->increment_ref_count();
        return rc;
      }

      /**
       * Converts to an immutable Rc<T>
       */
      template<typename Base>
      requires std::derived_from<T, Base>
      operator Rc<Base>() const { // NOLINT(*-explicit-constructor)
        return upcast<Base>();
      }

      /**
       * Implicitly coerce into a reference to the contained
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR operator T&() const { // NOLINT(*-explicit-constructor)
        return *raw_ptr();
      }

      /**
       * Implicitly coerce into a reference to a pointer to the contained value
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR operator T*() const { // NOLINT(*-explicit-constructor)
        return raw_ptr();
      }

      /**
       * Implicitly coerce into a reference to the contained
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR operator RefMut<T>() const { // NOLINT(*-explicit-constructor)
        return as_ref();
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto operator->() const -> T* {
        return raw_ptr();
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto operator*() const -> T& {
        return *raw_ptr();
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto operator->() -> T* {
        return raw_ptr();
      }

      CRAB_NODISCARD_INLINE_CONSTEXPR auto operator*() -> T& {
        return *raw_ptr();
      }

      /**
       * @brief Gets a const reference to the value inside.
       */
      CRAB_NODISCARD_INLINE_CONSTEXPR auto as_ref(const SourceLocation loc = SourceLocation::current()) const -> T& {
        return *raw_ptr(loc);
      }

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
        return not is_valid() ? 0 : get_interior()->get_ref_count();
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
      [[nodiscard]] auto get(const SourceLocation loc = SourceLocation::current()) const -> T& {
        return *raw_ptr(loc);
      }

      /**
       * @brief Returns a raw pointer to the underlying data
       */
      [[nodiscard]] auto raw_ptr(const SourceLocation loc = SourceLocation::current()) const -> T* {
        return get_interior(loc)->raw_ptr();
      }

      /**
       * @brief Gets raw pointer to the RcInterior, do not use this unless you have
       * a reason to mess with Rc directly
       */
      [[nodiscard]] auto get_interior(const SourceLocation loc = SourceLocation::current()) const -> Interior* {
        debug_assert_transparent(
          is_valid(),
          loc,
          "Invalid use of RcMut<T>, Interior is nullptr - this is most likely the "
          "result of a use-after-move."
        );
        return interior;
      }

      /**
       * @brief Gets raw pointer to the RcInterior, do not use this unless you have
       * a very good reason for messing with the invariants of Rc
       */
      [[nodiscard]] auto get_interior(const SourceLocation loc = SourceLocation::current()) -> Interior*& {
        debug_assert_transparent(
          is_valid(),
          loc,
          "Invalid use of RcMut<T>, Interior is nullptr - this is most likely the "
          "result of a use-after-move."
        );
        return interior;
      }

      /**
       * @brief  Attempts to take ownership of the shared value, this will only
       * suceed if this is the only reference to the instance.
       */
      [[nodiscard]] auto try_release(const SourceLocation loc = SourceLocation::current()) && -> Option<Box<T>> {
        return std::exchange(interior, nullptr)->release(loc);
      }
    };

    /**
     * Creates a new reference counted instance of T
     * @tparam T The type to be heap allocated & reference counted
     * @tparam Args Argument types to be passed to T's constructor
     * @param args Arguments to be passed to T's constructor
     */
    template<typename T, typename... Args>
    requires std::constructible_from<T, Args...>
    [[nodiscard]] auto make_rc(Args&&... args) -> Rc<T> {
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
    [[nodiscard]] auto make_rc_mut(Args&&... args) -> RcMut<T> {
      return RcMut<T>::from_owned_unchecked(new T{std::forward<Args>(args)...});
    }
  }

  using rc::make_rc;

  using rc::make_rc_mut;
} // namespace crab

namespace crab::opt {
  template<typename T, template<typename Inner> typename Container>
  struct RcStorage final {
    using RefCounted = Container<T>;

    CRAB_INLINE_CONSTEXPR explicit RcStorage(RefCounted value): inner{std::move(value)} {}

    CRAB_INLINE_CONSTEXPR explicit RcStorage(const opt::None& = crab::none): inner{nullptr} {}

    CRAB_INLINE_CONSTEXPR auto operator=(RefCounted&& value) -> RcStorage& {
      inner = std::move(value);
      return *this;
    }

    CRAB_INLINE_CONSTEXPR auto operator=(const opt::None&) -> RcStorage& {
      if (in_use()) {
        crab::discard(RefCounted{std::move(inner)});
      }
      return *this;
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto value() const& -> const RefCounted& {
      return inner;
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto value() & -> RefCounted& {
      return inner;
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto value() && -> RefCounted {
      return std::move(inner);
    }

    CRAB_NODISCARD_INLINE_CONSTEXPR auto in_use() const -> bool {
      return inner.is_valid();
    }

  private:

    RefCounted inner;
  };
}

namespace crab::prelude {
  using ::crab::rc::Rc;
  using ::crab::rc::RcMut;
}

CRAB_PRELUDE_GUARD;

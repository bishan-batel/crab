#pragma once

#include <type_traits>
#include "crab/assertion/check.hpp"
#include "crab/core.hpp"
#include "crab/core/cases.hpp"
#include "crab/core/discard.hpp"
#include "crab/mem/forward.hpp"
#include "crab/core/SourceLocation.hpp"

#include "crab/any/impl/AnyOfStorage.hpp"
#include "crab/any/impl/Buffer.hpp"
#include "crab/any/impl/visitor.hpp"
#include "crab/ty/identity.hpp"
#include "crab/opt/Option.hpp"

namespace crab::any {

  template<typename... Ts>
  class AnyOf final : private impl::AnyOfConstructor<Ts>... {
    static_assert(
      ((ty::movable<Ts> or ty::is_reference<Ts>) and ...),
      "Cannot construct an AnyOf with an immovable Type"
    );

    static_assert((ty::non_const<Ts> and ...), "Cannot have a variant of AnyOf be const");

    static_assert(
      (not ty::same_as<Ts, AnyOf> and ...),
      "Cannot have an AnyOf<Ts...> instantiate that recursively contains itself."
    );

    template<typename T>
    static constexpr auto _instances_of_type() -> usize {
      return ([]() { return ty::same_as<T, Ts> ? 1 : 0; }() + ...);
    }

    static_assert(((_instances_of_type<Ts>() == 1) and ...), "Cannot have duplicate types in an AnyOf");

  public:

    /**
     * The total number of different types
     */
    static constexpr auto NumTypes{sizeof...(Ts)};

    static_assert(NumTypes > 0, "Cannot create an AnyOf with novariants");

    /**
     * The minimum sized storage buffer required for storage
     */
    static constexpr usize DataSize{impl::ByteSize<Ts...>};

    /**
     * The minimum alignment required for storage
     */
    static constexpr usize Alignment{impl::AlignOf<Ts...>};

  private:

    template<ty::either<Ts...> T>
    using Storage = impl::AnyOfStorage<T>;

    template<typename F>
    static constexpr bool FunctorAcceptsMutRef{(ty::consumer<F, Ts&> or ...)};

    template<typename F>
    static constexpr bool FunctorAcceptsConstRef{(ty::consumer<F, const Ts&> or ...)};

    template<typename F>
    static constexpr bool FunctorAcceptsRvalue{(ty::consumer<F, Ts&&> or ...)};

  public:

    static_assert(
      static_cast<usize>(-1) >= NumTypes,
      "AnyOf does not support more than 255 variants, please reflect on the design of your program"
    );

    template<usize I>
    using NthType = ty::nth_type<I, Ts...>;

    template<ty::either<Ts...> T>
    static constexpr usize IndexOf = []() {
      usize i = 0;
      return ([&i]() {
        const bool is_index{ty::same_as<T, Ts>};
        i++;
        return is_index ? (i - 1) : 0;
      }() + ...);
    }();

  private:

    template<usize I>
    constexpr explicit AnyOf(const std::integral_constant<usize, I>&, NthType<I>&& value): index{I} {
      Storage<NthType<I>>::construct(buffer, mem::forward<NthType<I>>(value));
    }

    using impl::AnyOfConstructor<Ts>::impl_construct...;
    using impl::AnyOfConstructor<Ts>::impl_insert...;
    using impl::AnyOfConstructor<Ts>::impl_emplace...;

  public:

    // NOLINTBEGIN(*explicit*)
    template<typename T>
    constexpr AnyOf(T&& value) {
      const auto result{
        impl_construct(mem::forward<T>(value), buffer),
      };
      index = IndexOf<typename decltype(result)::type>;
    }

    constexpr AnyOf(AnyOf&& from) noexcept((std::is_nothrow_move_constructible_v<Ts> and ...)): index{from.index} {
      crab_check(from.is_valid(), "Cannot construct AnyOf from an invalid (moved-from) AnyOf");

      ([this, &from]() {
        if (index != IndexOf<Ts>) {
          return false;
        }

        Storage<Ts>::move(from.buffer, buffer);
        return true;
      } or ...);

      from.destroy();
      from.invalidate();
    }

    constexpr AnyOf(const AnyOf& from) requires(ty::copyable<Ts> and ...)
        : index{from.index} {
      crab_check(from.is_valid(), "Cannot construct AnyOf from an invalid (moved-from) AnyOf");

      static_assert(
        (ty::copyable<Ts> and ...),
        "Cannot copy construct an AnyOf<Ts...> that may contain a non copyable type."
      );

      ([this, &from]() {
        if (index != IndexOf<Ts>) {
          return false;
        }

        Storage<Ts>::copy(from.buffer, buffer);
        return true;
      } or ...);

      from.destroy();
      from.invalidate();
    }

    auto operator=(const AnyOf& from) -> AnyOf& {
      if (mem::address_of(from) == this) [[unlikely]] {
        return *this;
      }

      if (from.index == index) {
        ([this, &from]() {
          if (index != IndexOf<Ts>) {
            return false;
          }

          Storage<Ts>::copy_asign(from.buffer, buffer);
          return true;
        }()
         or ...);
        return *this;
      }

      destroy();
      index = from.index;

      ([this, &from]() {
        if (index != IndexOf<Ts>) {
          return false;
        }

        Storage<Ts>::copy(from.buffer, buffer);
        return true;
      }()
       or ...);

      return *this;
    }

    auto operator=(AnyOf&& from) noexcept -> AnyOf& {
      if (mem::address_of(from) == this) [[unlikely]] {
        return *this;
      }

      if (from.index == index) {
        ([this, &from]() {
          if (index != IndexOf<Ts>) {
            return false;
          }

          Storage<Ts>::move_assign(from.buffer, buffer);
          return true;
        }()
         or ...);

        from.destroy();
        from.invalidate();

        return *this;
      }

      destroy();
      index = from.index;

      ([this, &from]() {
        if (index != IndexOf<Ts>) {
          return false;
        }

        Storage<Ts>::move(from.buffer, buffer);
        return true;
      }()
       or ...);

      from.destroy();
      from.invalidate();

      return *this;
    }

    /**
     * Factor method for an AnyOf allowing explicit specification of the variant to emplace, useful if you have
     * an AnyOf<Ts...> where multiple types could be convertible to one another like references or scalars.
     */
    template<ty::either<Ts...> T>
    [[nodiscard]] static constexpr auto from(ty::identity<T> value) -> AnyOf {
      return AnyOf{
        std::integral_constant<usize, IndexOf<T>>{},
        mem::forward<T>(value),
      };
    }

    /**
     * Factor method for an AnyOf with explicit specification for the type by index (AnyOf<T0,T1,T2,...>)
     */
    template<usize N>
    [[nodiscard]] static constexpr auto from(NthType<N> value) -> AnyOf {
      static_assert(N < NumTypes, "Invalid type index passed to AnyOf::from<usize>");

      return AnyOf{
        std::integral_constant<usize, N>{},
        mem::forward<NthType<N>>(value),
      };
    }

    ~AnyOf() {
      if (not is_valid()) {
        return;
      }

      destroy();
    }

    // NOLINTEND(*explicit*)

    /**
     * Retrieves an optional const reference to the inner value if the current type is the one being asked for
     */
    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr auto as() const& -> opt::Option<const T&> {
      if (get_index() != IndexOf<T>) {
        return {};
      }

      return opt::Option<const T&>{as_unchecked<T>()};
    }

    /**
     * Retrieves a mutable const reference to the inner value if the current type is the one being asked for
     */
    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr auto as() & -> opt::Option<T&> {
      if (get_index() != IndexOf<T>) {
        return {};
      }

      return opt::Option<T&>{as_unchecked<T>()};
    }

    /**
     * If the requested type is the one contained, this will return an option with the value moved into, note that
     * even if this method returns none the storage inside will be destroyed.
     */
    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr auto as() && -> opt::Option<T> {
      if (get_index() != IndexOf<T>) {
        destroy();
        invalidate();
        return {};
      }

      return opt::Option<T>{mem::move(*this).template as_unchecked<T>()};
    }

    /**
     * Returns the reference type requested if the current value is of that type.
     */
    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr auto as() & -> opt::Option<T> {
      if (get_index() != IndexOf<T>) {
        return {};
      }

      return opt::Option<T>{as_unchecked<T>()};
    }

    /**
     * Returns the reference type requested if the current value is of that type.
     */
    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr auto as() const& -> opt::Option<T> {
      if (get_index() != IndexOf<T>) {
        return {};
      }

      return opt::Option<T>{as_unchecked<T>()};
    }

    /**
     * Returns the reference type requested if the current value is of that type.
     * Note that after this method is called, this AnyOf is left invalid (this is rvalue qualified)
     */
    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr auto as() && -> opt::Option<T> {
      if (get_index() != IndexOf<T>) {
        destroy();
        invalidate();
        return {};
      }

      return opt::Option<T>{mem::move(*this).template as_unchecked<T>()};
    }

    /**
     * Insert an existing value to replace the one stored
     *
     * This method can be safely called on an moved-from AnyOf, this implementation will always be the same as
     * operator=(T)
     */
    template<typename T>
    constexpr auto insert(T&& value) & -> void {
      static_assert(
        (ty::convertible<T, Ts> or ...),
        "Cannot insert with a type that is incompatible with all cases of this AnyOf"
      );

      if (is_valid()) {
        destroy();
      }

      auto result{
        this->impl_insert(mem::forward<T>(value), buffer),
      };

      index = IndexOf<typename decltype(result)::type>;
    }

    template<typename T>
    auto operator=(T&& value) -> AnyOf& {
      insert<T>(mem::forward<T>(value));
      return *this;
    }

    /**
     * Construct a value of the specified type to replace the one stored
     *
     * This method can be safely called on an moved-from AnyOf, this implementation will always be the same as
     * operator=(T)
     */
    template<ty::either<Ts...> T, typename... Args>
    constexpr auto emplace(Args&&... args) & -> void {
      if (is_valid()) {
        destroy();
      }

      this->impl_emplace(std::type_identity<T>{}, buffer, mem::forward<Args>(args)...);
      index = IndexOf<T>;
    }

  private:

    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked() & -> T& {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
    }

    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked() const& -> const T& {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
    }

    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked() && -> T {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");

      T value{mem::forward<T>(Storage<T>::as_ref(buffer))};

      destroy();
      invalidate();

      return value;
    }

    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked() & -> T {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
    }

    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked() const& -> T {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
    }

    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked() && -> T {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      T value{Storage<T>::as_ref(buffer)};

      // no destructor required, we know this is a reference

      invalidate();
      return value;
    }

  public:

    template<typename... Fs, typename R = impl::VisitorResultType<crab::cases<Fs...>, const Ts&...>>
    [[nodiscard]] constexpr auto match(Fs&&... functions) const& -> R {
      static_assert(sizeof...(Fs) != 0, "Must have at least one functor to match with");

      using Visitor = crab::cases<Fs...>;

      if constexpr (ty::not_void<R>) {
        return visit<Visitor, R>(Visitor{mem::forward<Fs>(functions)...});
      } else {
        visit<Visitor, R>(Visitor{mem::forward<Fs>(functions)...});
      }
    }

    template<typename... Fs, typename R = impl::VisitorResultType<crab::cases<Fs...>, Ts&...>>
    [[nodiscard]] constexpr auto match(Fs&&... functions) & -> R {
      static_assert(sizeof...(Fs) != 0, "Must have at least one functor to match with");

      using Visitor = crab::cases<Fs...>;

      if constexpr (ty::not_void<R>) {
        return visit<Visitor, R>(Visitor{mem::forward<Fs>(functions)...});
      } else {
        visit<Visitor, R>(Visitor{mem::forward<Fs>(functions)...});
      }
    }

    template<typename... Fs, typename R = impl::VisitorResultType<crab::cases<Fs...>, Ts&&...>>
    [[nodiscard]] constexpr auto match(Fs&&... functions) && -> R {
      static_assert(sizeof...(Fs) != 0, "Must have at least one functor to match with");

      using Visitor = crab::cases<Fs...>;

      if constexpr (ty::not_void<R>) {
        return std::move(*this).template visit<Visitor, R>(Visitor{mem::forward<Fs>(functions)...});
      } else {
        std::move(*this).template visit<Visitor, R>(Visitor{mem::forward<Fs>(functions)...});
      }
    }

    template<impl::VisitorForTypes<const Ts&...> Visitor, typename R = impl::VisitorResultType<Visitor, const Ts&...>>
    constexpr auto visit(Visitor&& visitor) const& -> R {

      // ensure visitor has a call overload that can accept any type contained in this AnyOf

      static_assert(
        impl::VisitorForTypes<Visitor, const Ts&...>,
        "Visitor must be able to accept all types that could be contained in an AnyOf<Ts...>"
      );

      using JumpTableFn = R (*)(const AnyOf& self, Visitor);

      const std::array<JumpTableFn, NumTypes> table{
        [](const AnyOf& self, Visitor visitor) -> R {
          const Ts& value{self.as_unchecked<Ts>()};

          if constexpr (ty::not_void<R>) {
            return std::invoke(visitor, value);
          } else {
            std::invoke(visitor, value);
          }
        }...,
      };

      if constexpr (ty::not_void<R>) {
        return table.at(get_index())(*this, mem::forward<Visitor>(visitor));
      } else {
        table.at(get_index())(*this, mem::forward<Visitor>(visitor));
      }
    }

    template<impl::VisitorForTypes<Ts&...> Visitor, typename R = impl::VisitorResultType<Visitor, Ts&...>>
    constexpr auto visit(Visitor&& visitor) & -> R {

      // ensure visitor has a call overload that can accept any type contained in this AnyOf

      static_assert(
        impl::VisitorForTypes<Visitor, Ts&...>,
        "Visitor must be able to accept all types that could be contained in an AnyOf<Ts...>"
      );

      using JumpTableFn = R (*)(AnyOf& self, Visitor);

      const std::array<JumpTableFn, NumTypes> table{
        [](AnyOf& self, Visitor visitor) -> R {
          Ts& value{self.as_unchecked<Ts>()};

          if constexpr (ty::not_void<R>) {
            return std::invoke(visitor, value);
          } else {
            std::invoke(visitor, value);
          }
        }...,
      };

      if constexpr (ty::not_void<R>) {
        return table.at(get_index())(*this, mem::forward<Visitor>(visitor));
      } else {
        table.at(get_index())(*this, mem::forward<Visitor>(visitor));
      }
    }

    template<impl::VisitorForTypes<Ts&&...> Visitor, typename R = impl::VisitorResultType<Visitor, Ts&&...>>
    constexpr auto visit(Visitor&& visitor) && -> R {

      // ensure visitor has a call overload that can accept any type contained in this AnyOf

      static_assert(
        impl::VisitorForTypes<Visitor, Ts&&...>,
        "Visitor must be able to accept all types that could be contained in an AnyOf<Ts...>"
      );

      // using JumpTableFn = Func<R(const impl::Buffer<Size, Alignment>&, Visitor)>;

      using JumpTableFn = R (*)(AnyOf self, Visitor);

      const std::array<JumpTableFn, NumTypes> table{
        [](AnyOf self, Visitor visitor) -> R {
          Ts& value{mem::move(self).template as_unchecked<Ts>()};

          if constexpr (ty::not_void<R>) {
            return std::invoke(visitor, value);
          } else {
            std::invoke(visitor, value);
          }
        }...,
      };

      if constexpr (ty::not_void<R>) {
        return table.at(get_index())(mem::move(*this), mem::forward<Visitor>(visitor));
      } else {
        table.at(get_index())(mem::move(*this), mem::forward<Visitor>(visitor));
      }
    }

    template<ty::either<Ts...> T>
    [[nodiscard]] constexpr auto is() const -> bool {
      return get_index() == IndexOf<T>;
    }

    [[nodiscard]] constexpr auto get_index() const -> usize {
      crab_check(is_valid(), "Invalid use of a moved-from AnyOf");
      CRAB_ASSUME(index < NumTypes);
      return index;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto is_valid() const -> bool {
      return index < NumTypes;
    }

  private:

    /**
     * Destroys interior
     */
    constexpr auto destroy() {
      crab::discard(([this]() -> bool {
        if (index == IndexOf<Ts>) {
          Storage<Ts>::destroy(buffer);
          return true;
        }
        return false;
      }() or ...));
    }

    /**
     * Marks this instance as invalid
     */
    constexpr auto invalidate() -> void {
      index = static_cast<u8>(-1);
    }

    impl::Buffer<DataSize, Alignment> buffer;
    u8 index;
  };
}

namespace crab::prelude {
  using any::AnyOf;
}

CRAB_PRELUDE_GUARD;

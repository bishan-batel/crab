/// @file crab/any/AnyOf
#pragma once

#include <type_traits>
#include "crab/core/unsafe.hpp"
#include "crab/assertion/check.hpp"
#include "crab/core.hpp"
#include "crab/core/cases.hpp"
#include "crab/core/discard.hpp"
#include "crab/mem/forward.hpp"
#include "crab/core/SourceLocation.hpp"

#include "crab/any/forward.hpp"

#include "crab/any/impl/AnyOfStorage.hpp"
#include "crab/any/impl/Buffer.hpp"
#include "crab/any/impl/visitor.hpp"
#include "crab/ty/identity.hpp"
#include "crab/opt/Option.hpp"

namespace crab::any {

  /// A heterogeneuous sum type loosely equivalent to std::variant
  template<typename... Ts>
  class AnyOf final : private impl::AnyOfConstructor<Ts>... {
    /// AnyOf would be significantly more complex to support non-moveable types, a case that is not worth it in my
    /// opinion.
    /// I would much rather value the invariant "all instantiations of AnyOf<Ts...> are moveable".
    static_assert(
      ((ty::movable<Ts> or ty::is_reference<Ts>) and ...),
      "Cannot construct an AnyOf with an immovable Type"
    );

    /// const-ness of AnyOf is transitive, note this does not apply to references.
    static_assert((ty::non_const<Ts> and ...), "Cannot have a variant of AnyOf be const");

    /// This it for double checking, but I don't believe there is a way to 'spell' this form of recursive instantiation
    /// other than inheritance hack (which is disallowed because AnyOf is final).
    static_assert(
      (not ty::same_as<Ts, AnyOf> and ...),
      "Cannot have an AnyOf<Ts...> instantiate that recursively contains itself."
    );

    /// Constexpr helper for counting the occurences of T to check for multiple occurences in the same AnyOf
    /// (which is not allowed).
    /// @internal
    template<typename T>
    static constexpr auto instances_of_type() -> usize {
      return ([]() { return ty::same_as<T, Ts> ? 1 : 0; }() + ...);
    }

    static_assert(((instances_of_type<Ts>() == 1) and ...), "Cannot have duplicate types in an AnyOf");

  public:

    /// The total number of different types
    static constexpr auto NumTypes{sizeof...(Ts)};

    /// AnyOf<> would always be invalid, therefore not allowed.
    static_assert(NumTypes > 0, "Cannot create an AnyOf with novariants");

    /// The minimum sized storage buffer required for storage
    static constexpr usize DataSize{impl::ByteSize<Ts...>};

    /// The minimum alignment required for storage
    static constexpr usize Alignment{impl::AlignOf<Ts...>};

  private:

    /// Alias for internal storage helper for each given type.
    /// @internal
    template<ty::either<Ts...> T>
    using Storage = impl::AnyOfStorage<T>;

  public:

    /// self explanatory
    static_assert(
      static_cast<usize>(-1) >= NumTypes,
      "AnyOf does not support more than 255 variants, please reflect on the design of your program"
    );

    /// Utility for indexing the 'Ts...' pack to get  the nth type in the list.
    ///
    /// This metafunction only works with values of I within the range [0, NumTypes), this does not allow indexing out
    /// of bounds of the type array.
    ///
    /// # Examples
    /// ```cpp
    /// using A = AnyOf<i32, u32>;
    ///
    /// static_assert(typeid(A::NthType<0>) == typeid(i32));
    /// static_assert(typeid(A::NthType<1>) == typeid(u32));
    ///
    /// // ill-formed
    /// // typeid(A::NthType<2>);
    /// ```
    template<usize I>
    using NthType = ty::nth_type<I, Ts...>;

    /// Utility for locationg the index of a given type in the Ts pack.
    ///
    /// This requires that the input type T must be one contained in the pack 'Ts...'.
    ///
    /// # Examples
    /// ```cpp
    /// using A = AnyOf<i32, u32>;
    ///
    /// static_assert(A::IndexOf<i32> == 0);
    /// static_assert(A::IndexOf<u32> == 1);
    ///
    /// // ill-formed
    /// // typeid(A::NthType<2>);
    /// ```
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

    /// Internal constructor with explicit selection of what type is being constructed. Due to the jank of this
    /// language, usage is required to be used with the factory method AnyOf::from.
    /// @internal
    template<usize I>
    constexpr explicit AnyOf(const std::integral_constant<usize, I>&, NthType<I>&& value): index{I} {
      Storage<NthType<I>>::construct(buffer, mem::forward<NthType<I>>(value));
    }

    /// Inherited helper for easier selection of value construction for a specific type
    /// @internal
    using impl::AnyOfConstructor<Ts>::impl_construct...;

    /// Inherited helper for easier selection of value construction for a specific type
    /// @internal
    using impl::AnyOfConstructor<Ts>::impl_insert...;

    /// Inherited helper for easier selection of value construction for a specific type
    /// @internal
    using impl::AnyOfConstructor<Ts>::impl_emplace...;

  public:

    /// Factory method for an AnyOf allowing explicit specification of the variant to emplace, useful if you have
    /// an AnyOf<Ts...> where multiple types could be convertible to one another like references or scalars.
    ///
    /// @tparam T The exact type this AnyOf must be constructed as
    /// @param value The value to place into the AnyOf being constructed
    /// @returns a value of AnyOf with the given value wrapped
    template<ty::either<Ts...> T>
    [[nodiscard]] static constexpr auto from(ty::identity<T> value) -> AnyOf {
      return AnyOf{
        std::integral_constant<usize, IndexOf<T>>{},
        mem::forward<T>(value),
      };
    }

    /// Factory method for an AnyOf with explicit specification for the type by index (AnyOf<T0,T1,T2,...>)
    ///
    /// @tparam N The index of the specific type being constructed (see NthType<I>).
    /// @param value The value to place into the AnyOf being constructed
    /// @returns a value of AnyOf with the given value wrapped
    template<usize N>
    [[nodiscard]] static constexpr auto from(NthType<N> value) -> AnyOf {
      static_assert(N < NumTypes, "Invalid type index passed to AnyOf::from<usize>");

      return AnyOf{
        std::integral_constant<usize, N>{},
        mem::forward<NthType<N>>(value),
      };
    }

    /// Construct an AnyOf with the given value inside. This value must be compatible with one of the values of this
    /// AnyOf can contain. Note that is can be sometimes unclear what variant is being constructed.
    ///
    /// To ensure that the correct type is being constructed, use the factory method AnyOf::from.
    /// Note these concerns are not as needed if the types contained are not subtypes of one another.
    ///
    /// For example, in this case it would be clearer to use the from factory method.
    ///
    /// ```cpp
    /// const u32 a = 10;
    /// u32 b = 20;
    /// AnyOf<u32& ,const u32&> v = a;
    /// ```
    template<typename T>
    constexpr AnyOf(T&& value) /* NOLINT, silence warnings about hiding forward constructors */ {

      /// SFINAE guard to prevent this constructor from hiding AnyOf's move constructor
      static_assert(ty::different_than<T, AnyOf>);
      const auto result{
        impl_construct(mem::forward<T>(value), buffer),
      };
      index = IndexOf<typename decltype(result)::type>;
    }

    /// Move constructor
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

    /// Copy constructor, only valid if all instances of AnyOf are valid.
    constexpr AnyOf(const AnyOf& from) requires(ty::copyable<Ts> and ...)
        : index{from.index} {

      /// for optimisations with certain API's, this function allows construction from moved-from AnyOf's.
      // crab_check(from.is_valid(), "Cannot construct AnyOf from an invalid (moved-from) AnyOf");

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
    }

    /// Copy assignment, only valid of all variants Ts... are copyable.
    auto operator=(const AnyOf& from) -> AnyOf& requires(ty::copyable<Ts> and ...)
    {
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

    /// Move assignment
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

    /// Destructor
    ~AnyOf() {
      if (not is_valid()) {
        return;
      }

      destroy();
    }

    // NOLINTEND(*explicit*)

    /// Retrieves an optional const reference to the inner value if the current type is the one being asked for
    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr auto as() const& -> opt::Option<const T&> {
      if (get_index() != IndexOf<T>) {
        return {};
      }

      // SAFETY: as_unchecked valid, we just validated the index.
      return opt::Option<const T&>{as_unchecked<T>(unsafe)};
    }

    /// Retrieves a mutable const reference to the inner value if the current type is the one being asked for
    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr auto as() & -> opt::Option<T&> {
      if (get_index() != IndexOf<T>) {
        return {};
      }

      // SAFETY: as_unchecked valid, we just validated the index.
      return opt::Option<T&>{as_unchecked<T>(unsafe)};
    }

    /// If the requested type is the one contained, this will return an option with the value moved into, note that
    /// even if this method returns none the storage inside will be destroyed.
    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr auto as() && -> opt::Option<T> {
      if (get_index() != IndexOf<T>) {
        destroy();
        invalidate();
        return {};
      }

      // SAFETY: as_unchecked valid, we just validated the index.
      return opt::Option<T>{mem::move(*this).template as_unchecked<T>(unsafe)};
    }

    /// Returns the reference type requested if the current value is of that type.
    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr auto as() & -> opt::Option<T> {
      if (get_index() != IndexOf<T>) {
        return {};
      }

      // SAFETY: as_unchecked valid, we just validated the index.
      return opt::Option<T>{as_unchecked<T>(unsafe)};
    }

    ///
    /// Returns the reference type requested if the current value is of that type.
    ///
    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr auto as() const& -> opt::Option<T> {
      if (get_index() != IndexOf<T>) {
        return {};
      }

      // SAFETY: as_unchecked valid, we just validated the index.
      return opt::Option<T>{as_unchecked<T>(unsafe)};
    }

    /// Returns the reference type requested if the current value is of that type.
    /// note that after this method is called, this AnyOf is left invalid (this is rvalue qualified)
    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr auto as() && -> opt::Option<T> {
      if (get_index() != IndexOf<T>) {
        destroy();
        invalidate();
        return {};
      }

      // SAFETY: as_unchecked valid, we just validated the index.
      return opt::Option<T>{mem::move(*this).template as_unchecked<T>(unsafe)};
    }

    /// Insert an existing value to replace the one stored
    ///
    /// This method can be safely called on an moved-from AnyOf, this implementation will always be the same as
    /// operator=(T)
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

    /// Construct a value of the specified type to replace the one stored
    ///
    /// This method can be safely called on an moved-from AnyOf, this implementation will always be the same as
    /// operator=(T)
    template<ty::either<Ts...> T, typename... Args>
    constexpr auto emplace(Args&&... args) & -> void {
      if (is_valid()) {
        destroy();
      }

      this->impl_emplace(std::type_identity<T>{}, buffer, mem::forward<Args>(args)...);
      index = IndexOf<T>;
    }

    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked(unsafe_fn) & -> T& {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
    }

    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked(unsafe_fn) const& -> const T& {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
    }

    template<ty::either<Ts...> T>
    requires ty::non_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked(unsafe_fn) && -> T {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");

      T value{mem::forward<T>(Storage<T>::as_ref(buffer))};

      destroy();
      invalidate();

      return value;
    }

    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked(unsafe_fn) & -> T {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
    }

    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked(unsafe_fn) const& -> T {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
    }

    template<ty::either<Ts...> T>
    requires ty::is_reference<T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked(unsafe_fn) && -> T {
      crab_dbg_check(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      T value{Storage<T>::as_ref(buffer)};

      // no destructor required, we know this is a reference

      invalidate();
      return value;
    }

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
          const Ts& value{self.as_unchecked<Ts>(unsafe)};

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
          Ts& value{self.as_unchecked<Ts>(unsafe)};

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
          Ts& value{mem::move(self).template as_unchecked<Ts>(unsafe)};

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

    /// Returns whether this AnyOf is in a valid state.
    /// This method should not be something you anchor design around, instead of relying on weaker invariance you should
    /// be adding it via the type system using a Option<AnyOf<Ts...>>
    ///
    /// This method has no preconditions for the state of AnyOf, and unlike others can be called on an instance of AnyOf
    /// in a moved-from state.
    [[nodiscard]] CRAB_INLINE constexpr auto is_valid() const -> bool {
      return index < NumTypes;
    }

  private:

    /// Destroys interior
    /// @interior
    constexpr auto destroy() {
      crab::discard(([this]() -> bool {
        if (index == IndexOf<Ts>) {
          Storage<Ts>::destroy(buffer);
          return true;
        }
        return false;
      }() or ...));
    }

    /// Marks this instance as invalid
    /// @interior
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

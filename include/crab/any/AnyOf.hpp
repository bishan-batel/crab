#pragma once

#include <type_traits>
#include <variant>
#include "crab/assertion/assert.hpp"
#include "crab/core.hpp"
#include "crab/core/cases.hpp"
#include "crab/core/discard.hpp"
#include "crab/fn/Func.hpp"
#include "crab/mem/forward.hpp"
#include "crab/core/SourceLocation.hpp"
#include "crab/mem/size_of.hpp"
#include "crab/type_traits.hpp"

#include "crab/any/impl/AnyOfStorage.hpp"
#include "crab/any/impl/Buffer.hpp"
#include "crab/any/impl/visitor.hpp"

namespace crab::any {

  template<typename... Ts>
  class AnyOf final : private impl::AnyOfConstructor<Ts>... {
    static_assert(
      ((ty::movable<Ts> or ty::is_reference<Ts>) and ...),
      "Cannot construct an AnyOf with an immovable Type"
    );
    static_assert((ty::non_const<Ts> and ...), "Cannot have a variant of AnyOf be const");

  public:

    /**
     * The total number of different types
     */
    static constexpr auto NumTypes{sizeof...(Ts)};

    static_assert(NumTypes > 0, "Cannot create an AnyOf with novariants");

    /**
     * The minimum sized storage buffer required for storage
     */
    static constexpr usize Size{impl::ByteSize<Ts...>};

    /**
     * The minimum alignment required for storage
     */
    static constexpr usize Alignment{impl::AlignOf<Ts...>};

  private:

    template<ty::either<Ts...> T>
    using Storage = impl::AnyOfStorage<T>;

    template<typename F>
    static constexpr bool FunctorAcceptsMutRef = (ty::consumer<F, Ts&> or ...);

    template<typename F>
    static constexpr bool FunctorAcceptsConstRef = (ty::consumer<F, const Ts&> or ...);

    template<typename F>
    static constexpr bool FunctorAcceptsRvalue = (ty::consumer<F, Ts&&> or ...);

  public:

    template<usize I>
    using NthType = ty::nth_type<I, Ts...>;

    template<ty::either<Ts...> T>
    static constexpr usize IndexOf = []() {
      usize i{0};

      return ([&i]() {
        const bool is_index{ty::same_as<T, Ts>};
        i++;
        return is_index ? (i - 1) : 0;
      }() + ...);
    }();

    AnyOf(AnyOf&& from) = delete;

    AnyOf(const AnyOf& from) = delete;

    auto operator=(AnyOf&& from) -> AnyOf& = delete;

    auto operator=(const AnyOf& from) -> const AnyOf& = delete;

  private:

    template<usize I>
    constexpr explicit AnyOf(const std::integral_constant<usize, I>&, NthType<I>&& value): index{I} {
      Storage<NthType<I>>::construct(buffer, mem::forward<NthType<I>>(value));
    }

  public:

    // NOLINTBEGIN(*explicit*)
    template<typename T>
    constexpr AnyOf(T&& value) {
      const auto& result{
        impl_construct(mem::forward<T>(value), buffer),
      };
      index = IndexOf<typename std::remove_cvref_t<decltype(result)>::type>;
    }

    template<ty::either<Ts...> T>
    [[nodiscard]] static constexpr auto from(T&& value) -> AnyOf {
      return AnyOf{
        std::integral_constant<usize, IndexOf<T>>{},
        mem::forward<T>(value),
      };
    }

  private:

    using impl::AnyOfConstructor<Ts>::impl_construct...;

  public:

    ~AnyOf() {
      if (not is_valid()) {
        return;
      }

      destruct();
    }

    // NOLINTEND(*explicit*)

    template<ty::either<Ts...> T>
    [[nodiscard]] constexpr auto as() const -> opt::Option<const T&> {
      ensure_valid();

      if (get_index() != IndexOf<T>) {
        return {};
      }

      return opt::Option<const T&>{as_unchecked<T>()};
    }

    template<ty::either<Ts...> T>
    [[nodiscard]] constexpr auto as() -> opt::Option<T&> {
      ensure_valid();

      if (get_index() != IndexOf<T>) {
        return {};
      }

      return opt::Option<T&>{as_unchecked<T>()};
    }

  private:

    template<ty::either<Ts...> T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked() -> T& {
      debug_assert(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
    }

    template<ty::either<Ts...> T>
    [[nodiscard]] constexpr CRAB_INLINE auto as_unchecked() const -> const T& {
      debug_assert(get_index() == IndexOf<T>, "as_unchecked<T> called on AnyOf that does not contain T");
      return Storage<T>::as_ref(buffer);
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

    template<impl::VisitorForTypes<const Ts&...> Visitor, typename R = impl::VisitorResultType<Visitor, const Ts&...>>
    constexpr auto visit(Visitor&& visitor) const& -> R {

      // ensure visitor has a call overload that can accept any type contained in this AnyOf

      static_assert(
        impl::VisitorForTypes<Visitor, const Ts&...>,
        "Visitor must be able to accept all types that could be contained in an AnyOf<Ts...>"
      );

      // calling visit on a moved-from AnyOf is ill-formed
      ensure_valid();

      // using JumpTableFn = Func<R(const impl::Buffer<Size, Alignment>&, Visitor)>;

      using JumpTableFn = R (*)(const AnyOf& self, Visitor);

      std::array<JumpTableFn, NumTypes> table{
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
        return table.at(index)(*this, mem::forward<Visitor>(visitor));
      } else {
        table.at(index)(*this, mem::forward<Visitor>(visitor));
      }
    }

    template<impl::VisitorForTypes<Ts&...> Visitor, typename R = impl::VisitorResultType<Visitor, Ts&...>>
    constexpr auto visit(Visitor&& visitor) & -> R {

      // ensure visitor has a call overload that can accept any type contained in this AnyOf

      static_assert(
        impl::VisitorForTypes<Visitor, Ts&...>,
        "Visitor must be able to accept all types that could be contained in an AnyOf<Ts...>"
      );

      // calling visit on a moved-from AnyOf is ill-formed
      ensure_valid();

      // using JumpTableFn = Func<R(const impl::Buffer<Size, Alignment>&, Visitor)>;

      using JumpTableFn = R (*)(AnyOf& self, Visitor);

      std::array<JumpTableFn, NumTypes> table{
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
        return table.at(index)(*this, mem::forward<Visitor>(visitor));
      } else {
        table.at(index)(*this, mem::forward<Visitor>(visitor));
      }
    }

    // TODO: rvalue qualified visit, match

    template<ty::either<Ts...> T>
    [[nodiscard]] constexpr auto is() const -> bool {
      return get_index() == IndexOf<T>;
    }

    [[nodiscard]] constexpr auto get_index() const -> usize {
      ensure_valid();
      CRAB_ASSUME(index < NumTypes);
      return index;
    }

    [[nodiscard]] CRAB_INLINE constexpr auto is_valid() const -> usize {
      return index < NumTypes;
    }

  private:

    constexpr auto destruct() {
      crab::discard(([this]() -> bool {
        if (is<Ts>()) {
          Storage<Ts>::destroy(buffer);
          return true;
        }
        return false;
      }() or ...));
    }

    constexpr auto invalidate() {
      index = static_cast<usize>(-1);
    }

    CRAB_INLINE constexpr auto ensure_valid(const SourceLocation& loc = SourceLocation::current()) const -> void {
      debug_assert_transparent(is_valid(), loc, "Invalid use of a moved-from AnyOf");
    }

    impl::Buffer<Size, Alignment> buffer;
    usize index;
  };
}

namespace crab::prelude {
  using any::AnyOf;
}

CRAB_PRELUDE_GUARD;

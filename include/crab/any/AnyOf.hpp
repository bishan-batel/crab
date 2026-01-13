#pragma once

#include <variant>
#include "crab/assertion/assert.hpp"
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
  class AnyOf final {
    static_assert((ty::movable<Ts> and ...), "Cannot construct an AnyOf with an immovable Type");
    static_assert((ty::non_const<Ts> and ...), "Cannot have a variant of AnyOf be const");

    /**
     * The total number of different types
     */
    static constexpr auto NumTypes{sizeof...(Ts)};

    static_assert(NumTypes > 0, "Cannot create an AnyOf with novariants");

    /**
     * The minimum sized storage buffer required for storage
     */
    static constexpr usize Size{
      std::max({mem::size_of<Ts>()...}),
    };

    /**
     * The minimum alignment required for storage
     */
    static constexpr usize Alignment{
      std::max({alignof(Ts)...}),
    };

    template<ty::either<Ts...> T>
    using Storage = impl::AnyOfStorage<T>;

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

    template<typename F>
    static constexpr bool FunctorAcceptsMutRef = (ty::consumer<F, Ts&> or ...);

    template<typename F>
    static constexpr bool FunctorAcceptsConstRef = (ty::consumer<F, const Ts&> or ...);

    template<typename F>
    static constexpr bool FunctorAcceptsRvalue = (ty::consumer<F, Ts&&> or ...);

    AnyOf(AnyOf&& from) = delete;

    AnyOf(const AnyOf& from) = delete;

    auto operator=(AnyOf&& from) -> AnyOf& = delete;

    auto operator=(const AnyOf& from) -> const AnyOf& = delete;

    // NOLINTBEGIN(*explicit*)
    template<ty::either<Ts...> T>
    AnyOf(T&& value): index{IndexOf<T>} {
      Storage<T>::construct(buffer, mem::forward<T>(value));
    }

    ~AnyOf() {
      if (not is_valid()) {
        return;
      }

      destruct();
    }

    // NOLINTEND(*explicit*)

    template<ty::either<Ts...> T>
    [[nodiscard]] constexpr auto as() -> opt::Option<const T&> {
      ensure_valid();

      if (get_index() != IndexOf<T>) {
        return {};
      }

      return opt::Option<const T&>{Storage<T>::as_ref(buffer)};
    }

    template<ty::either<Ts...> T>
    [[nodiscard]] constexpr auto as_mut() -> opt::Option<T&> {
      ensure_valid();

      if (get_index() != IndexOf<T>) {
        return {};
      }

      return opt::Option<T&>{Storage<T>::as_ref(buffer)};
    }

    template<typename... Fs>
    [[nodiscard]] constexpr auto match(Fs&&... functions) const& -> decltype(auto) {
      static_assert(sizeof...(Fs) != 0, "Must have at least one functor to match with");

      using R = decltype(visit(
        crab::cases{
          mem::forward<Fs>(functions)...,
        }
      ));

      static_assert(ty::not_void<R>);
      return visit(
        crab::cases{
          mem::forward<Fs>(functions)...,
        }
      );
    }

    template<typename... Fs>
    constexpr auto match(Fs&&... functions) const& -> void {
      static_assert(sizeof...(Fs) != 0, "Must have at least one functor to match with");

      using R = decltype(visit(
        crab::cases{
          mem::forward<Fs>(functions)...,
        }
      ));

      static_assert(ty::same_as<R, void>);

      visit(
        crab::cases{
          mem::forward<Fs>(functions)...,
        }
      );
    }

    template<impl::VisitorForTypes<const Ts&...> Visitor, typename R = impl::VisitorResultType<Visitor, const Ts&...>>
    constexpr auto visit(Visitor&& visitor) const& -> R {
      static_assert(
        impl::VisitorForTypes<Visitor, const Ts&...>,
        "Visitor must be able to accept all types that could be contained in an AnyOf<Ts...>"
      );

      ensure_valid();

      std::visit(visitor);

      constexpr std::array<Func<R(const AnyOf&)>, NumTypes> table{
        [](const impl::Buffer<Size, Alignment>& buffer, Visitor&& visitor) {
          const Ts& value{Storage<Ts>::as_ref(buffer)};

          if constexpr (ty::not_void<R>) {
            return std::invoke(visitor, value);
          } else {
            std::invoke(visitor, value);
          }
        }...
      };

      if constexpr (ty::not_void<R>) {
        return std::invoke(table.at(index), buffer, mem::forward<Visitor>(visitor));
      } else {
        std::invoke(table.at(index), buffer, mem::forward<Visitor>(visitor));
      }
    }

    template<ty::either<Ts...> T>
    [[nodiscard]] constexpr auto is() const -> bool {
      return get_index() == IndexOf<T>;
    }

    [[nodiscard]] constexpr auto get_index() const -> usize {
      ensure_valid();

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

    CRAB_INLINE constexpr auto ensure_valid(const SourceLocation& loc = SourceLocation::current()) const -> void {
      debug_assert_transparent(is_valid(), loc, "Invalid use of a moved-from AnyOf");
    }

    impl::Buffer<Size, Alignment> buffer;
    usize index;
  };
}

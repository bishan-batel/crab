#pragma once

#include <crab/preamble.hpp>
#include <crab/ref.hpp>
#include <crab/option.hpp>
#include <type_traits>

namespace crab {

  namespace ref {
    /**
     * Converts a T* to a optional reference (these two types are isomorphic)
     */
    template<typename T>
    [[nodiscard]] constexpr auto from_ptr(T* const from) -> Option<T&> {
      return crab::then(from != nullptr, [from]() -> T& { return *from; });
    }

    /**
     * Converts a const T* to a optional reference (these two types are
     * isomorphic)
     */
    template<typename T>
    [[nodiscard]]
    constexpr auto from_ptr(const T* const from) -> Option<const T&> {
      return crab::then(from != nullptr, [from]() -> const T& {
        return *from;
      });
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    [[nodiscard]]
    constexpr auto cast(const Base& from) -> Option<const Derived&> {
      return from_ptr(dynamic_cast<const Derived*>(&from));
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    [[nodiscard]]
    constexpr auto cast(Base& from) -> Option<Derived&> {
      return from_ptr(dynamic_cast<Derived*>(&from));
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    [[nodiscard]]
    constexpr auto cast(const Base* from) -> Option<const Derived&> {
      return from_ptr(dynamic_cast<const Derived*>(from));
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    [[nodiscard]] constexpr auto cast(Base* from) -> Option<Derived&> {
      return from_ptr(dynamic_cast<Derived*>(from));
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    [[nodiscard]] constexpr auto cast(Ref<Base> from) -> Option<Ref<Derived>> {
      return cast<Derived, Base>(*from).template map<Ref<Derived>>();
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    [[nodiscard]] constexpr Option<RefMut<Derived>> cast(RefMut<Base> from) {
      return cast<Derived, Base>(*from).template map<RefMut<Derived>>();
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    [[nodiscard]]
    constexpr auto cast(Option<Ref<Base>> from) -> Option<Ref<Derived>> {
      return from.flat_map([](const auto& base) {
        return cast<Derived, Base>(base);
      });
    }

    /**
     * @brief Attempts to cast input of type Base into a Derived instances
     */
    template<class Derived, std::derived_from<Derived> Base>
    [[nodiscard]]
    constexpr auto cast(Option<RefMut<Base>> from
    ) noexcept -> Option<Ref<Derived>> {
      return from.flat_map([](const auto& base) {
        return cast<Derived, Base>(base);
      });
    }

    /**
     * @brief Checks if the given object is an instance of some type
     *
     * @tparam Derived What type to check
     * @param obj Object to check
     */
    template<class Derived, std::derived_from<Derived> Base>
    [[nodiscard]] constexpr auto is(const Base& obj) noexcept -> bool {
      return dynamic_cast<const Derived*>(&obj) != nullptr;
    }

    /**
     * @brief Is this given parameter *exactly* this type.
     *
     * This will not perform a recursive check like dynamic_cast
     *
     * ```cpp
     *
     * class A {};
     *
     * class B: public A {};
     *
     * class C: public B {};
     *
     * i32 main() {
     *   A a;
     *   B b;
     *
     *   debug_assert(crab::ref::is_exact<A>(a),"");
     *   debug_assert(crab::ref::is_exact<B>(b),"");
     *
     *   debug_assert(crab::ref::is<A>(c),"");
     *   debug_assert(not crab::ref::is_exact<A>(c),"");
     * }
     *
     *
     * ```
     *
     * @tparam Derived
     * @param obj
     * @return
     */
    template<typename T>
    [[nodiscard]] constexpr auto is_exact(const auto& obj) noexcept -> bool {
      return typeid(obj) == typeid(T);
    }
  }

  /**
   * Explicit way of coercing a value to implicitly cast to a type rather than
   * using static_cast
   */
  template<typename T>
  [[nodiscard]] auto implicit_cast( //
    std::type_identity_t<T> type
  ) noexcept(std::is_nothrow_move_constructible_v<T>) -> T {
    return type;
  }

} // namespace crab::ref

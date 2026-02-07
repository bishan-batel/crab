/// @file crab/opt/concepts.hpp
/// @ingroup opt

#pragma once

#include <concepts>
#include "crab/opt/forward.hpp"
#include "crab/ty/bool_types.hpp"
#include "crab/ty/classify.hpp"
#include "crab/ty/compare.hpp"
#include "crab/ty/construct.hpp"

namespace crab::opt {
  namespace impl {
    /// Type predicate helper for if the given type T is an option type, false
    /// for most types
    /// @ingroup opt
    /// @internal
    template<typename>
    struct is_option_type final : ty::false_type {};

    /// Type predicate helper for if the given type is an option type, true
    /// only if the given type is of the form Option<T> for some T
    /// @ingroup opt
    /// @internal
    template<typename T>
    struct is_option_type<opt::Option<T>> final : ty::true_type {};

  }

  /// Type predicate for if the given type T is some form of crab Option
  /// @ingroup opt
  /// @hideinitializer
  template<typename T>
  concept option_type = impl::is_option_type<T>::value;

  /// Type trait to help impose restrictions for custom Option storage types, note that just because your type satisfies
  /// this does not mean it is guarenteed to work, this is simply to establish base assumptions & prereqs. The type
  /// system in C++ is not strong enough to guarentee that what you do here is not ill-formed even if you satisfy all
  /// these requirements on a surface level.
  ///
  /// Note most of these assumptions are null when talking about storing a reference. While possible, it is discouraged
  /// to overload the storage type for any reference type and leave it to its default storage provided by crab for all
  /// Option<T&>.
  /// The major difference between storage for reference types compared to any other is that the 'value()' method does
  /// not need to have varying return types depending on if the given storage is const, lvalue, or rvalue.
  ///
  /// Specifically these requirements.
  /// ```cpp
  /// { std::declval<const Storage&>().value() } -> ty::same_as<const T&>;
  /// { std::declval<Storage&>().value()       } -> ty::same_as<T&>;
  /// { std::declval<Storage&&>().value()      } -> ty::same_as<T>;
  /// ```
  ///
  /// In C++ it is impoossible to have a reference to a reference, therefore in the case of reference types it makes
  /// more sense to always return the same reference, which could be marked as:
  ///
  /// ```cpp
  /// { std::declval<const Storage<T&>&>().value() } -> ty::same_as<T&>;
  /// { std::declval<Storage<T&>&>().value()       } -> ty::same_as<T&>;
  /// { std::declval<Storage<T&>&&>().value()      } -> ty::same_as<T&>;
  /// ```
  ///
  /// @ingroup opt
  template<typename Storage, typename T>
  concept is_storage_type = requires {
    // the storage type should be default constructible, eg. ideally this should be the same as constructing from
    // 'None'
    requires ty::default_constructible<Storage>;

    // supports construction from the contained type
    requires std::constructible_from<Storage, T&&>;

    // supprts construction for an empty value
    requires std::constructible_from<Storage, None>;

    // supports moving around at the bare minimum
    requires ty::movable<Storage>;

    // can assign to be a value
    requires std::is_assignable_v<Storage, T&&>;

    // can assign to be empty
    requires std::is_assignable_v<Storage, None>;

    // in_use should return a bool and work for any reference type to Storage
    { std::declval<const Storage&>().in_use() } -> ty::same_as<bool>;
    { std::declval<Storage&>().in_use() } -> ty::same_as<bool>;
    { std::declval<Storage&&>().in_use() } -> ty::same_as<bool>;
    // requirements that are except for reference types.
    { std::declval<const Storage&>().value() } -> ty::same_as<const T&>;
    { std::declval<Storage&>().value() } -> ty::same_as<T&>;
    { std::declval<Storage&&>().value() } -> ty::same_as<T>;
  } or ty::is_reference<T>;

}

#pragma once

#include <array>
#include <utility>

#include "crab/boxed/Box.hpp"
#include "crab/ty/classify.hpp"
#include "crab/mem/move.hpp"
#include "crab/mem/address_of.hpp"
#include "crab/mem/size_of.hpp"
#include "crab/mem/copy.hpp"

namespace crab::mem::impl {
  /// Helper to swap two items
  template<ty::non_const T>
  constexpr auto swap_non_trivial(T& lhs, T& rhs) -> void {
    // call std::swap due to possible optimised specializations for certain types
    std::swap(lhs, rhs);
  }

  /// Helper to swap two items that are trivially relocatable.
  /// Note this function uses mem::copy therefore cannot be constexpr.
  ///
  /// @internal
  template<ty::non_const T>
  auto swap_trivial_relocatable(unsafe_fn, T& lhs, T& rhs) -> void {
    // TODO: implement trivial relocatability
    std::array<u8, mem::size_of<T>()> temp;

    // copy bytes of lhs to temp
    mem::copy(unsafe, mem::address_of(lhs), reinterpret_cast<T*>(temp.data()), 1);

    // copy bytes of rhs to lhs
    mem::copy(unsafe, mem::address_of(rhs), mem::address_of(lhs), 1);

    // copy temp back into rhs
    mem::copy(unsafe, reinterpret_cast<T*>(temp.data()), mem::address_of(rhs), 1);
  }

};

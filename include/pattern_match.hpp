//
// Created by bishan_ on 4/24/24.
//

#pragma once

#include <sys/cdefs.h>
#include <variant>
#include "preamble.hpp"

#include "option.hpp"
#include "result.hpp"

// namespace crab {
//   namespace pattern::meta {
//
//     template<typename Eval, typename F, typename... Variants>
//     constexpr __always_inline auto match_case(const F &functor, Variants &...variants) -> Option<Eval>;
//   } // namespace pattern::meta
//
//   template<typename Visitor, typename... Variants>
//   constexpr auto match(Visitor, Variants...) -> void;
// } // namespace crab
//
// template<typename Eval, typename F, typename... Variants>
// constexpr __always_inline auto crab::pattern::meta::match_case(const F &, Variants &...) -> Option<Eval> {
//   return crab::none;
// }
//
// template<typename Visitor, typename... Variants>
// constexpr auto crab::match(Visitor, Variants...) -> void {}

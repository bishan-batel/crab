/// @file crab/any/any.hpp
#pragma once

/// @defgroup any Any (Discriminated Unions)

/// @namespace crab::any
/// @ingroup any
///
/// This namespace is primarily for the AnyOf<Ts...> type, which is a discriminated
/// union type analogous to std::variant with an API that does not use exceptions,
/// as well as the invariant of being non-null (unless in a moved-from state, in
/// which case AnyOf is only safe to be destroyed or reassigned)
namespace crab::any {}

/// @file ty.hpp
/// crab::ty's namespace documentation.

#pragma once

/// @defgroup ty Metatemplate Helpers & Constraints
/// @{

/// @namespace crab::ty
/// @ingroup ty
///
/// Crab's meta-template helper fundamentals. This namespace contains mainly concepts and other type-metafunctions. Many
/// of the concepts in this namespace are synonyms to the STL, but aimed to leverage more readable metatemplate code.
///
/// One of the biggest differences in crab's implementations is the emphasis on the values produced by concepts and
/// helper structs rather than the jank helper structs themselves. Many STL concepts exist but many have yet to be
/// created (like std::is_array_v/std::is_array).
namespace crab::ty {}

/// }@

/// @file crab/term/term.hpp
#pragma once

/// @namespace crab::term
///
/// The crab::term namespace is sparce due to it containing things that are on the edge of what crab is really for,
/// the main use of this namespace are helpers for manipulating terminal interfaces in an OS-independent way (for ex.
/// enabling ANSI color codes)
///
/// This is not meant to be a comprehensive collection of all terminal controls, crab does not
/// aim to fill that niche, this is mainly used for colored debug message when the program is
/// in a (crab) paniced state.
namespace crab::term {}

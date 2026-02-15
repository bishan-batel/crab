/// @file crab/preamble.hpp
/// Convenience header used by users of crab that imports common headers & alias.
/// Note that this header will NEVER be included by another crab header.

#pragma once

#include "crab/core.hpp"

#include <utility>
#include <array>
#include <span>
#include <ranges>

#include "crab/core/SourceLocation.hpp"
#include "crab/core/cases.hpp"
#include "crab/core/discard.hpp"
#include "crab/core/unit.hpp"
#include "crab/core/unreachable.hpp"
#include "crab/core/unsafe.hpp"
#include "crab/core.hpp"

#include "crab/num/floating.hpp"
#include "crab/num/integer.hpp"
#include "crab/num/num.hpp"
#include "crab/num/range.hpp"
#include "crab/num/suffixes.hpp"

#include "crab/str/str.hpp"

#include "crab/ty/bool_types.hpp"
#include "crab/ty/classify.hpp"
#include "crab/ty/compare.hpp"
#include "crab/ty/construct.hpp"
#include "crab/ty/crab_ref_decay.hpp"
#include "crab/ty/functor.hpp"
#include "crab/ty/identity.hpp"
#include "crab/ty/manipulate.hpp"
#include "crab/ty/ty.hpp"

#include "crab/assertion/assert.hpp"
#include "crab/assertion/check.hpp"
#include "crab/assertion/fmt.hpp"
#include "crab/assertion/forward.hpp"
#include "crab/assertion/panic.hpp"
#include "crab/assertion/todo.hpp"

#include "crab/mem/address_of.hpp"
#include "crab/mem/copy.hpp"
#include "crab/mem/forward.hpp"
#include "crab/mem/mem.hpp"
#include "crab/mem/move.hpp"
#include "crab/mem/replace.hpp"
#include "crab/mem/size_of.hpp"
#include "crab/mem/swap.hpp"
#include "crab/mem/take.hpp"

#include "crab/convert/From.hpp"

#include "crab/env/env.hpp"

#include "crab/collections/Dictionary.hpp"
#include "crab/collections/Set.hpp"
#include "crab/collections/Tuple.hpp"
#include "crab/collections/Vec.hpp"

#include "crab/fn/Func.hpp"
#include "crab/fn/cast.hpp"
#include "crab/fn/fn.hpp"
#include "crab/fn/identity.hpp"

#include "crab/hash/hash.hpp"

#include "crab/ops/Add.hpp"
#include "crab/ops/Div.hpp"
#include "crab/ops/Mul.hpp"
#include "crab/ops/Rem.hpp"
#include "crab/ops/Sub.hpp"
#include "crab/ops/ops.hpp"

#include "crab/opt/Option.hpp"
#include "crab/opt/boolean_constructs.hpp"
#include "crab/opt/concepts.hpp"
#include "crab/opt/fallible.hpp"
#include "crab/opt/forward.hpp"
#include "crab/opt/none.hpp"
#include "crab/opt/opt.hpp"
#include "crab/opt/some.hpp"

#include "crab/ref/casts.hpp"
#include "crab/ref/forward.hpp"
#include "crab/ref/from_ptr.hpp"
#include "crab/ref/implicit_cast.hpp"
#include "crab/ref/is.hpp"
#include "crab/ref/is_exact.hpp"
#include "crab/ref/ref.hpp"

#include "crab/boxed/Box.hpp"
#include "crab/boxed/boxed.hpp"
#include "crab/boxed/forward.hpp"

#include "crab/any/AnyOf.hpp"
#include "crab/any/any.hpp"
#include "crab/any/forward.hpp"

#include "crab/rc/Rc.hpp"

#include "crab/result/Err.hpp"
#include "crab/result/Error.hpp"
#include "crab/result/Ok.hpp"
#include "crab/result/Result.hpp"
#include "crab/result/concepts.hpp"
#include "crab/result/fallible.hpp"
#include "crab/result/forward.hpp"
#include "crab/result/unwrap.hpp"

#include "crab/term/Handle.hpp"
#include "crab/term/ansi.hpp"
#include "crab/term/forward.hpp"
#include "crab/term/term.hpp"

namespace crab {
  /// Alias for std::pair.
  template<typename A, typename B>
  using Pair = std::pair<A, B>;

  /// Alias for std::array.
  /// Statically Sized list of packed objects
  template<typename T, usize length>
  using SizedArray = std::array<T, length>;

  /// Alias for std::span.
  /// This is an abstraction over any contiguous sequence of elements / slice.
  template<typename T, usize length = std::dynamic_extent>
  using Span = std::span<T, length>;

}

namespace crab::prelude {

  using ::crab::Pair;

#if !CRAB_NO_TYPEDEF_ARRAY
  using ::crab::SizedArray;
#endif

#if !CRAB_NO_TYPEDEF_SPAN
  using ::crab::Span;
#endif

  /// std::ranges Alias
  namespace ranges = ::std::ranges;

  /// std::ranges::views Alias
  namespace views = ::std::ranges::views;
}

CRAB_PRELUDE_GUARD;

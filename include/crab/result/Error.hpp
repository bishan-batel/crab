/// @file crab/result/Error.hpp
/// @ingroup result
#pragma once

#include <concepts>
#include <stdexcept>
#include "crab/boxed/forward.hpp"
#include "crab/rc/Rc.hpp"
#include "crab/result/concepts.hpp"
#include "crab/assertion/fmt.hpp"
#include "crab/str/str.hpp"

namespace crab::result {
  /// Base error type for use with Result<T, E>
  /// @relates IError
  /// @ingroup result
  class IError {
  public:

    IError() = default;

    IError(const IError&) = default;

    IError(IError&&) noexcept = default;

    IError& operator=(const IError&) = default;

    IError& operator=(IError&&) noexcept = default;

    virtual ~IError() = default;

    /// Stringified error message for logging purposes
    [[nodiscard]] virtual auto what() const -> String = 0;
  };
}

/// Specialization to support formatting any type derived from IError
/// @ingroup result
template<std::derived_from<crab::result::IError> T, typename Char>
struct fmt::formatter<T, Char> : fmt::formatter<String> {
  constexpr auto format(const crab::result::IError& error, format_context& ctx) const {
    return formatter<String>::format(error.what(), ctx);
  }
};

namespace crab {
  using result::IError;
}

#pragma once

#include <stdexcept>
#include "crab/boxed/forward.hpp"
#include "crab/rc/Rc.hpp"
#include "crab/result/concepts.hpp"
#include "crab/assertion/fmt.hpp"
#include "crab/str/str.hpp"

namespace crab::result {
  /// @addtogroup result
  /// @{

  /// Base error type for use with Result<T, E>
  /// @relates IError
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

  /// @relates IError
  template<typename E>
  [[nodiscard]] auto error_reason(const E& error) -> String {
    return crab::to_string(error);
  }

  /// @relates IError
  [[nodiscard]] CRAB_INLINE auto error_reason(const IError& error) -> String {
    return error.what();
  }

  /// @relates IError
  template<std::derived_from<IError> E>
  [[nodiscard]] constexpr auto error_reason(const E& error) -> String {
    return error.what();
  }

  /// @relates IError
  template<typename E>
  [[nodiscard]] constexpr auto error_reason(const boxed::Box<E>& error) -> String {
    return error_reason(*error);
  }

  /// @relates IError
  template<typename E>
  [[nodiscard]] constexpr auto error_reason(const rc::Rc<E>& error) -> String {
    return error_reason(*error);
  }

  /// @relates IError
  template<typename E>
  [[nodiscard]] constexpr auto error_reason(const rc::RcMut<E>& error) -> String {
    return error_reason(*error);
  }

  /// }@
}

namespace crab {
  using result::IError;
}

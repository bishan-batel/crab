#pragma once

#include <stdexcept>
#include "crab/result/concepts.hpp"
#include "crab/assertion/fmt.hpp"
#include "crab/str/str.hpp"

namespace crab {
  namespace result {
    /**
     * @brief Base error type for use with Result<T, E>
     */
    class Error {
    public:

      Error() = default;

      Error(const Error&) = default;

      Error(Error&&) noexcept = default;

      Error& operator=(const Error&) = default;

      Error& operator=(Error&&) noexcept = default;

      virtual ~Error() = default;

      /**
       * @brief Converts this crab Error into a runtime exception that can be
       * thrown, if needed when dealing with certain API's
       *
       * @return
       */
      [[nodiscard]] auto as_exception() const -> std::runtime_error {
        return std::runtime_error{what()};
      }

      /**
       * @brief Stringified error message for logging purposes
       */
      [[nodiscard]] virtual auto what() const -> String = 0;
    };

    /**
     * @brief Converts a given error to its stringified representation.
     */
    template<error_type E>
    [[nodiscard]] constexpr auto error_to_string(const E& err) {
      if constexpr (requires {
                      { err.what() } -> crab::ty::convertible<String>;
                    }) {
        return err.what();
      }

      else if constexpr (requires {
                           { err->what() } -> crab::ty::convertible<String>;
                         }) {
        return err->what();
      }

      else {
        return crab::to_string(err);
      }
    }

  }

  using result::Error;
}

#pragma once

#include <utility>
#include "preamble.hpp"

namespace ex {

  class MoveOnly {
  public:

    constexpr explicit MoveOnly(String name): name{std::move(name)} {}

    constexpr MoveOnly(const MoveOnly&) = delete;

    constexpr MoveOnly(MoveOnly&& from) noexcept: name{std::move(from.name)} {}

    constexpr auto operator=(const MoveOnly&) -> MoveOnly& = delete;

    constexpr auto operator=(MoveOnly&& from) noexcept -> MoveOnly& {
      if (this == &from) {
        return *this;
      }

      name = std::move(from.name);

      return *this;
    }

    constexpr auto set_name(String name) -> void {
      this->name = std::move(name);
    }

    [[nodiscard]] constexpr auto get_name() const -> const String& {
      return name;
    }

  private:

    String name;
  };

  struct Copyable {
    constexpr explicit Copyable(String name): name{std::move(name)} {}

    constexpr auto set_name(String name) -> void {
      this->name = std::move(name);
    }

    [[nodiscard]] constexpr auto get_name() const -> const String& {
      return name;
    }

  private:

    String name;
  };

  struct Base {
    virtual ~Base() = default;

    [[nodiscard]] virtual auto name() const -> StringView { return "Base"; }
  };

  struct Derived : public Base {
    [[nodiscard]] auto name() const -> StringView override { return "Derived"; }
  };

  constexpr auto test_values =
    []<typename... T>(const auto& test, T&&... types) {
      const auto test_wrapped = [test]<typename S>(S&& x) {
        test(std::forward<S>(x));
        return unit::val;
      };
      (std::ignore = ... = test_wrapped(std::forward<T>(types)));
    };
}

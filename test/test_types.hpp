#pragma once

#include <utility>
#include "preamble.hpp"

class MoveOnlyType {
public:

  constexpr explicit MoveOnlyType(String name): name{std::move(name)} {}

  constexpr MoveOnlyType(const MoveOnlyType&) = delete;

  constexpr MoveOnlyType(MoveOnlyType&& from) noexcept:
      name{std::move(from.name)} {}

  constexpr auto operator=(const MoveOnlyType&) -> MoveOnlyType& = delete;

  constexpr auto operator=(MoveOnlyType&& from) noexcept -> MoveOnlyType& {
    if (this == &from) {
      return *this;
    }

    name = std::move(from.name);

    return *this;
  }

  constexpr auto set_name(String name) -> void { this->name = std::move(name); }

  [[nodiscard]] constexpr auto get_name() const -> const String& {
    return name;
  }

private:

  String name;
};

struct CopyableType {
  constexpr explicit CopyableType(String name): name{std::move(name)} {}

  constexpr auto set_name(String name) -> void { this->name = std::move(name); }

  [[nodiscard]] constexpr auto get_name() const -> const String& {
    return name;
  }

private:

  String name;
};

#pragma once

#include <catch2/catch_test_macros.hpp>
#include <preamble.hpp>
#include "option.hpp"

class MoveOnlyType {
public:

  MoveOnlyType(const MoveOnlyType&) = delete;

  MoveOnlyType(MoveOnlyType&& from) noexcept: name{std::move(from.name)} {}

  auto operator=(const MoveOnlyType&) -> MoveOnlyType& = delete;

  auto operator=(MoveOnlyType&& from) noexcept -> MoveOnlyType& {
    if (this == &from) {
      return *this;
    }

    name = std::move(from.name);

    return *this;
  }

  auto set_name(String name) -> void { this->name = std::move(name); }

  [[nodiscard]] auto get_name() const -> const String& { return name; }

private:

  String name;
};

struct CopyableType {
  explicit CopyableType(String name): name{std::move(name)} {}

  String name;
};

TEST_CASE("Monadic Operations") {
  Option<MoveOnlyType> type;
}

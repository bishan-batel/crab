#pragma once

#include <utility>
#include <crab/preamble.hpp>
#include <crab/ref/ref.hpp>
#include "crab/rc/Rc.hpp"
#include "test_static_asserts.hpp"

struct MoveCount {
  usize moves{0};
  usize copies{0};

  auto valid(const MoveCount& expected, std::source_location loc = std::source_location::current()) const -> void {
    do {
#if 0
      (void)__builtin_constant_p(moves == expected.moves);
#endif
      Catch ::AssertionHandler catchAssertionHandler(
        "CHECK"_catch_sr,
        ::Catch ::SourceLineInfo(loc.file_name(), loc.line()),
        "moves == expected.moves"_catch_sr,
        Catch ::ResultDisposition ::ContinueOnFailure
      );
      try {
        catchAssertionHandler.handleExpr((Catch ::Decomposer() <= moves) == expected.moves);
      } catch (...) {
        (catchAssertionHandler).handleUnexpectedInflightException();
      }
      catchAssertionHandler.complete();
    } while ((void)0, (false) && static_cast<const bool&>(!!(moves == expected.moves)));

    do {
#if 0
      (void)__builtin_constant_p(copies == expected.copies);
#endif
      Catch ::AssertionHandler catchAssertionHandler(
        "CHECK"_catch_sr,
        ::Catch ::SourceLineInfo(loc.file_name(), loc.line()),
        "copies == expected.copies"_catch_sr,
        Catch ::ResultDisposition ::ContinueOnFailure
      );
      try {
        catchAssertionHandler.handleExpr((Catch ::Decomposer() <= copies) == expected.copies);
      } catch (...) {
        (catchAssertionHandler).handleUnexpectedInflightException();
      }
      catchAssertionHandler.complete();
    } while ((void)0, (false) && static_cast<const bool&>(!!(copies == expected.copies)));
  }
};

/**
 * @brief Move semantic counter
 */
template<typename T>
class MoveTracker {
  static constexpr bool copyable = std::copyable<T> and std::is_copy_assignable_v<T>;

private:

  template<typename... Args>
  explicit MoveTracker(Args&&... value, RcMut<MoveCount> count):
      value{std::forward<Args>(value)...}, count{std::move(count)} {}

public:

  template<typename... Args>
  [[nodiscard]] static auto from(RcMut<MoveCount> count, Args&&... value) -> MoveTracker {
    return MoveTracker<T>(std::forward<Args>(value)..., std::move(count));
  }

  MoveTracker(MoveTracker&& from) noexcept: value{std::move(from.value)}, count{from.count} {
    count->moves++;
  }

  MoveTracker(const MoveTracker& from) noexcept requires copyable
      : value{from.value}, count{from.count} {
    count->copies++;
  }

  auto operator=(const MoveTracker& from) -> MoveTracker& requires copyable
  {
    if (&from == this) {
      return *this;
    }

    value = from.value;
    count = from.count;
    count->copies++;

    return *this;
  }

  auto operator=(MoveTracker&& from) noexcept -> MoveTracker& {
    if (&from == this) {
      return *this;
    }

    value = std::move(from.value);
    count = from.count;
    count->moves++;

    return *this;
  }

  [[nodiscard]] auto inner() -> T& {
    return value;
  }

  [[nodiscard]] auto inner() const -> const T& {
    return value;
  }

private:

  T value;
  RcMut<MoveCount> count;
};

class MoveOnly {
public:

  constexpr MoveOnly(): MoveOnly{""} {}

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

  constexpr Copyable(): Copyable{""} {}

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

  [[nodiscard]] virtual auto name() const -> StringView {
    return "Base";
  }
};

struct Derived final : public Base {
  [[nodiscard]] auto name() const -> StringView override {
    return "Derived";
  }
};

constexpr auto test_values = []<typename... T>(const auto& test, T&&... types) {
  const auto test_wrapped = [test]<typename S>(S&& x) {
    test(std::forward<S>(x));
    return unit::val;
  };
  (std::ignore = ... = test_wrapped(std::forward<T>(types)));
};

namespace asserts {
  constexpr auto common_types =
    asserts::types<i8, i32, i64, u8, u32, u64, usize, String, StringView, MoveOnly, Copyable>;

  template<typename T>
  constexpr auto cvref_supertypes = asserts::
    types<T, T*, const T*, T&, const T&, volatile T, volatile T*, volatile const T*, volatile T&, volatile const T&>;

  template<typename T>
  constexpr auto ref_types = asserts::types<Ref<T>, RefMut<T>, T&, const T&>;
}

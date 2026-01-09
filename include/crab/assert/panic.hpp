#pragma once

#include <iostream>
#include <stdexcept>
#include "crab/mem/move.hpp"
#include "crab/preamble.hpp"
#include "crab/fmt.hpp"

#include "execinfo.h"

namespace crab::assert {

  static constexpr usize MAX_BACKTRACE_STACKFRAMES{512};

  using PanicHook = Func<void(String)>;

  struct panic_handler final {
    panic_handler() = delete;

    static auto reset() -> void {
      handler = trivial_handler;
    }

    static auto set(PanicHook new_handler) -> void {
      handler = mem::move(new_handler);

      if (not handler) {
        reset();
      }
    }

    [[nodiscard]] static auto get() -> const PanicHook& {
      return handler;
    }

  private:

    static auto trivial_handler(String msg) -> void {
      std::cerr << msg << std::endl;
      // std::abort();
      throw std::runtime_error{mem::move(msg)};
    }

    inline static PanicHook handler{trivial_handler};
  };

  CRAB_NORETURN inline auto panic(StringView msg, const SourceLocation& loc = SourceLocation::current()) -> void {

    OutStringStream stream;

    stream << crab::format(
      "Panic occured at:\n {}:{}: in {} \n'{}'",
      loc.file_name(),
      loc.line(),
      loc.column(),
      loc.function_name()
    ) << '\n';

    stream << "Message: " << msg << '\n';

    stream << "Backtrace: " << msg << '\n';

#if CRAB_UNIX
    SizedArray<void*, MAX_BACKTRACE_STACKFRAMES> buffer{};

    backtrace(buffer.data(), buffer.size());

    usize count = 0;

    for (const void* v: buffer) {
      if (v == nullptr) {
        continue;
      }

      count++;
    }

    Span<char*> symbols{backtrace_symbols(buffer.data(), static_cast<int>(count)), count};

    if (not symbols.empty()) {
      for (const char* symbol: symbols) {
        if (symbol == nullptr) {
          break;
        }

        stream << "\t " << symbol << '\n';
      }

      free(symbols.data());
    } else {
      stream << "\t Failed to get backtrace" << '\n';
    }
#else
    stream << "\t Failed to get backtrace" << '\n';
#endif

    panic_handler::get()(stream.str());

    std::abort();
    unreachable();
  }

}

namespace crab {
  using crab::assert::panic;
}

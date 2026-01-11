#pragma once

#include <iostream>
#include <stdexcept>

#include "crab/preamble.hpp"

#include "crab/mem/move.hpp"
#include "crab/term/ansi.hpp"

namespace crab::assertion {

  static constexpr usize MAX_BACKTRACE_STACKFRAMES{512};

  struct PanicInfo final {
    String message;
    SourceLocation location;
  };

  using PanicHook = Func<void(PanicInfo)>;

  struct panic_handler final {
    panic_handler() = delete;

    inline static auto reset() -> void {
      handler = trivial_handler;
    }

    inline static auto set(PanicHook new_handler) -> void {
      handler = mem::move(new_handler);

      if (not handler) {
        reset();
      }
    }

    [[nodiscard]] inline static auto get() -> const PanicHook& {
      return handler;
    }

  private:

    inline static auto log_panic_to_stream(std::ostream& stream, bool should_color, const PanicInfo& msg) {
      [[maybe_unused]] static constexpr StringView ansii_red_bold{"\033[1;31m"};
      [[maybe_unused]] static constexpr StringView ansii_blue{"\033[0;34m"};
      [[maybe_unused]] static constexpr StringView ansii_green{"\033[0;32m"};
      [[maybe_unused]] static constexpr StringView ansii_reset{"\033[0m"};
      [[maybe_unused]] static constexpr StringView ansii_white_bold{"\33[1;37m"};

      const auto ansii{[&stream, should_color](StringView ansi) {
        if (should_color) {
          stream << ansi;
        }
      }};

      stream << '\n';
      ansii(ansii_red_bold);

      stream << "Panic:      ";

      ansii(ansii_green);

      stream << msg.message << '\n';

      // ansii(ansii_white_bold);
      ansii(ansii_blue);
      stream << "Ocurred at: ";

      stream << msg.location.file_name();
      stream << ":";
      stream << msg.location.line();
      stream << ": in ";
      stream << msg.location.column();
      stream << " Inside function '";
      stream << msg.location.function_name();
      stream << "'\n";

#if false
#if CRAB_UNIX 
    ansii(ansii_white_bold);

    stream << std::setw(15);
    stream << "Backtrace: " << '\n';

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
#endif

      ansii(ansii_reset);
      stream << '\n';
    }

    inline static auto trivial_handler(PanicInfo info) -> void {
#if CRAB_THROW_ON_DEFAULT_PANIC
      throw std::runtime_error{mem::move(info.message)};
#else
      log_panic_to_stream( //
        std::cerr,         //
        term::try_enable_ansi(term::Handle::Error),
        info
      );
      std::abort();
#endif
    }

    inline static PanicHook handler{trivial_handler};
  };

  CRAB_NORETURN inline auto panic(PanicInfo info) -> void {
    std::invoke(panic_handler::get(), info);
    unreachable();
  }

  CRAB_NORETURN inline auto panic(String msg, SourceLocation loc) -> void {
    panic(PanicInfo{
      mem::move(msg),
      loc,
    });
  }
}

namespace crab {}

#pragma once

// Logger. Backed by spdlog inside log.cpp. The header takes only POD types
// (no <format>/<string_view>) so TUs that mix `import std;` with `#include
// "log.hpp"` don't end up redeclaring std symbols — clang+libc++ rejects
// that combination loudly. Use the BLOCK_LOG_* macros for std::format-style
// call sites; the macros expand inside the caller's TU where `std::format`
// is already in scope (via `import std;`).

namespace block {

class Log {
 public:
  static void Init();
};

namespace log {

void Trace(const char* msg);
void Debug(const char* msg);
void Info(const char* msg);
void Warn(const char* msg);
void Error(const char* msg);
void Fatal(const char* msg);

}  // namespace log
}  // namespace block

#define BLOCK_LOG_TRACE(...) ::block::log::Trace(::std::format(__VA_ARGS__).c_str())
#define BLOCK_LOG_DEBUG(...) ::block::log::Debug(::std::format(__VA_ARGS__).c_str())
#define BLOCK_LOG_INFO(...) ::block::log::Info(::std::format(__VA_ARGS__).c_str())
#define BLOCK_LOG_WARN(...) ::block::log::Warn(::std::format(__VA_ARGS__).c_str())
#define BLOCK_LOG_ERROR(...) ::block::log::Error(::std::format(__VA_ARGS__).c_str())
#define BLOCK_LOG_FATAL(...) ::block::log::Fatal(::std::format(__VA_ARGS__).c_str())

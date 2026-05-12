#pragma once

// Logger. Backed by spdlog inside log.cpp. The header takes only POD types
// (no <format>/<string_view>) so TUs that mix `import std;` with `#include
// "log.hpp"` don't end up redeclaring std symbols — clang+libc++ rejects
// that combination loudly. Use the CK_LOG_* macros for std::format-style
// call sites; the macros expand inside the caller's TU where `std::format`
// is already in scope (via `import std;`).

namespace ck {

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
}  // namespace ck

#define CK_LOG_TRACE(...) ::ck::log::Trace(::std::format(__VA_ARGS__).c_str())
#define CK_LOG_DEBUG(...) ::ck::log::Debug(::std::format(__VA_ARGS__).c_str())
#define CK_LOG_INFO(...) ::ck::log::Info(::std::format(__VA_ARGS__).c_str())
#define CK_LOG_WARN(...) ::ck::log::Warn(::std::format(__VA_ARGS__).c_str())
#define CK_LOG_ERROR(...) ::ck::log::Error(::std::format(__VA_ARGS__).c_str())
#define CK_LOG_FATAL(...) ::ck::log::Fatal(::std::format(__VA_ARGS__).c_str())

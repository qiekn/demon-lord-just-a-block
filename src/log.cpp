#include "log.hpp"

#include <memory>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace ck {

namespace {

std::shared_ptr<spdlog::logger> g_logger;

}  // namespace

void Log::Init() {
  // %T -> HH:MM:SS, %^...%$ wraps the level-colored region, %l -> level tag.
  // Single shared logger; engine and gameplay share it.
  spdlog::set_pattern("%^[%T] [%l] %v%$");
  g_logger = spdlog::stdout_color_mt("block");
  g_logger->set_level(spdlog::level::trace);
}

namespace log {

void Trace(const char* msg) { g_logger->trace(msg); }
void Debug(const char* msg) { g_logger->debug(msg); }
void Info(const char* msg) { g_logger->info(msg); }
void Warn(const char* msg) { g_logger->warn(msg); }
void Error(const char* msg) { g_logger->error(msg); }
void Fatal(const char* msg) { g_logger->critical(msg); }

}  // namespace log
}  // namespace ck

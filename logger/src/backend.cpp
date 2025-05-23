#include "tgbm_replace/backend.hpp"

#include "tgbm_replace/ansi_style.hpp"
#include "tgbm_replace/config.hpp"
#include "tgbm_replace/scoped_file.hpp"

#include <fcntl.h>
#include <thread>

namespace tgbm::log {
namespace {

constexpr std::string_view debug_color = TGBM_ANSI_GREEN;
constexpr std::string_view info_color = TGBM_ANSI_BLUE;
constexpr std::string_view warning_color = TGBM_ANSI_YELLOW;
constexpr std::string_view error_color = TGBM_ANSI_RED;
constexpr std::string_view crit_color = TGBM_ANSI_BRIGHT_RED TGBM_ANSI_BOLD;
constexpr std::string_view trace_color = TGBM_ANSI_BRIGHT_CYAN;
constexpr std::string_view reset_color = TGBM_ANSI_RESET;

std::string_view level_to_color(level lvl) {
  using enum level;
  switch (lvl) {
    case debug:
      return debug_color;
    case info:
      return info_color;
    case warn:
      return warning_color;
    case error:
      return error_color;
    case crit:
      return crit_color;
    case trace:
      return trace_color;
    case level::_off:
      return reset_color;
  }
  std::terminate();
}

std::string console_fmt_colored(level lvl, std::string_view str) {
  return std::format("{}[{}]{:<10} {}{}\n", level_to_color(lvl), ts_str_now(),
                     prefix(lvl), str, reset_color);
}

std::string console_fmt(level lvl, std::string_view str) {
  return std::format("[{}]{:<10} {}{}\n", ts_str_now(), prefix(lvl), str, reset_color);
}

std::string thread_path() {
  std::stringstream ss;
  ss << std::this_thread::get_id();
  return std::format("{}.thread_log", ss.str());
}
}  // namespace

namespace internal {
scoped_file& get_thread_file() {
  thread_local scoped_file persistent_file(::fopen(thread_path().c_str(), "a"));
  return persistent_file;
}

scoped_file& get_unite_file() {
  thread_local scoped_file persistent_file(::fopen("unite.log", "a"));
  return persistent_file;
}
}  // namespace internal

void sink::std_out_colored(level lvl, std::string_view str) noexcept {
  std::string line = console_fmt_colored(lvl, str);
  size_t n = fwrite(line.data(), line.size(), 1, stdout);
  if (n != 1)
    std::terminate();
}

void sink::std_err_colored(level lvl, std::string_view str) noexcept {
  std::string line = console_fmt_colored(lvl, str);
  size_t n = fwrite(line.data(), line.size(), 1, stderr);
  if (n != 1)
    std::terminate();
}

void sink::std_out(level lvl, std::string_view str) noexcept {
  std::string line = console_fmt(lvl, str);
  size_t n = fwrite(line.data(), line.size(), 1, stdout);
  if (n != 1)
    std::terminate();
}

void sink::std_err(level lvl, std::string_view str) noexcept {
  std::string line = console_fmt(lvl, str);
  size_t n = fwrite(line.data(), line.size(), 1, stderr);
  if (n != 1)
    std::terminate();
}

void sink::per_thread_file(level lvl, std::string_view str) noexcept {
  std::string_view ts_s = ts_str_now();
  std::string_view p = prefix(lvl);
  internal::get_thread_file().write(ts_s).write(p).write(" ").write(str).write("\n");
}

void sink::unite_file(level lvl, std::string_view str) noexcept {
  auto line = std::string(ts_str_now());
  line.append(prefix(lvl)).append(" ").append(str).append("\n");
  internal::get_unite_file().write(line);
}

void sink::per_thread_file_with_flush(level lvl, std::string_view str) noexcept {
  sink::per_thread_file(lvl, str);
  internal::get_thread_file().flush();
}

void sink::unite_file_with_flush(level lvl, std::string_view str) noexcept {
  sink::unite_file(lvl, str);
  internal::get_unite_file().flush();
}
}  // namespace tgbm::log

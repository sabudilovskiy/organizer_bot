#pragma once

#include <exception>
#include <string_view>

namespace tgbm::log {

enum struct level : int {
  trace,
  debug,
  info,
  warn,
  error,
  crit,
  _off,
};

constexpr std::string_view to_string(level lvl) {
  switch (lvl) {
    case level::trace:
      return "trace";
    case level::debug:
      return "debug";
    case level::info:
      return "info";
    case level::warn:
      return "warn";
    case level::error:
      return "error";
    case level::crit:
      return "crit";
    case level::_off:
      return "off";
  }
  std::terminate();
}

constexpr std::string_view prefix(level lvl) {
  switch (lvl) {
    case level::trace:
      return "[trace]";
    case level::debug:
      return "[debug]";
    case level::info:
      return "[info]";
    case level::warn:
      return "[warn]";
    case level::error:
      return "[error]";
    case level::crit:
      return "[crit]";
    case level::_off:
      return "[off]";
  }
  std::terminate();
}

}  // namespace tgbm::log

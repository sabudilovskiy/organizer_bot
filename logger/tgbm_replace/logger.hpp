#pragma once

#include <fmt/format.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <format>
#include <iostream>

namespace bot::log {
constexpr auto INFO_colour = fmt::rgb(66, 125, 236);
constexpr auto DEBUG_colour = fmt::rgb(27, 124, 0);
constexpr auto WARN_colour = fmt::rgb(210, 185, 20);
constexpr auto CRIT_colour = fmt::rgb(255, 2, 2);
constexpr auto ERROR_colour = fmt::rgb(114, 5, 5);

inline auto zoned_now() {
  return std::chrono::zoned_time{std::chrono::current_zone(),
                                 std::chrono::system_clock::now()}
      .get_local_time();
}

inline void log_zoned_now() {
  ::std::format_to(std::ostreambuf_iterator{std::cout}, "[{:%H:%M:%S}]",
                   ::bot::log::zoned_now());
}

}  // namespace bot::log

#define TGBM_LOG_NOW() ::bot::log::log_zoned_now()

#define TGBM_LOG_IMPL(FMT_STR, LEVEL, ...)                     \
  TGBM_LOG_NOW(), ::fmt::print(fg(::bot::log::LEVEL##_colour), \
                               "[" #LEVEL "] " FMT_STR "\n" __VA_OPT__(, ) __VA_ARGS__)

#define TGBM_LOG(FMT_STR, ...) \
  TGBM_LOG_NOW(), ::fmt::print(FMT_STR "\n" __VA_OPT__(, ) __VA_ARGS__)

#define TGBM_LOG_WARN(FMT_STR, ...) \
  TGBM_LOG_IMPL(FMT_STR, WARN __VA_OPT__(, ) __VA_ARGS__)

#define TGBM_LOG_INFO(FMT_STR, ...) \
  TGBM_LOG_IMPL(FMT_STR, INFO __VA_OPT__(, ) __VA_ARGS__)

#define TGBM_LOG_DEBUG(FMT_STR, ...) \
  TGBM_LOG_IMPL(FMT_STR, DEBUG __VA_OPT__(, ) __VA_ARGS__)

#define TGBM_LOG_ERROR(FMT_STR, ...) \
  TGBM_LOG_IMPL(FMT_STR, ERROR __VA_OPT__(, ) __VA_ARGS__)

#define TGBM_LOG_CRIT(FMT_STR, ...) \
  TGBM_LOG_IMPL(FMT_STR, CRIT __VA_OPT__(, ) __VA_ARGS__)

#ifndef TGBM_HTTP2_LOG
  #define TGBM_HTTP2_LOG(TYPE, STR, ...)
#endif

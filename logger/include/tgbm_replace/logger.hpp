#pragma once

#include <fmt/format.h>

#include "tgbm_replace/config.hpp"

namespace tgbm::log {

template <typename... Args>
void log(level lvl, fmt::format_string<Args...> fmt, Args&&... args) {
  get_direct_sink_fn()(lvl, fmt::format(fmt, std::forward<Args>(args)...));
}

template <level LVL>
constexpr bool ct_should_log() {
  return TGBM_MIN_LOG_LEVEL <= LVL;
}

inline bool rt_should_log(level lvl) {
  return ::tgbm::log::get_log_level() <= lvl;
}

}  // namespace tgbm::log

#define TGBM_LOG(LEVEL, FMT, ...)                                                    \
  do {                                                                               \
    if constexpr (::tgbm::log::ct_should_log<::tgbm::log::level::LEVEL>())           \
      if (rt_should_log(::tgbm::log::level::LEVEL))                                  \
        ::tgbm::log::log(::tgbm::log::level::LEVEL, FMT __VA_OPT__(, ) __VA_ARGS__); \
  } while (0)

// clang-format off
#define TGBM_LOG_TRACE(FMT, ...) TGBM_LOG(trace, FMT __VA_OPT__(, ) __VA_ARGS__)
#define TGBM_LOG_DEBUG(FMT, ...) TGBM_LOG(debug, FMT __VA_OPT__(, ) __VA_ARGS__)
#define TGBM_LOG_INFO(FMT, ...)  TGBM_LOG(info, FMT __VA_OPT__(, ) __VA_ARGS__)
#define TGBM_LOG_WARN(FMT, ...)  TGBM_LOG(warn, FMT __VA_OPT__(, ) __VA_ARGS__)
#define TGBM_LOG_ERROR(FMT, ...) TGBM_LOG(error, FMT __VA_OPT__(, ) __VA_ARGS__)
#define TGBM_LOG_CRIT(FMT, ...) TGBM_LOG(crit, FMT __VA_OPT__(, ) __VA_ARGS__)
// clang-format on

#define TGBM_VLOG(LEVEL, FMT, ...)                             \
  do {                                                         \
    if (rt_should_log(LEVEL))                                  \
      ::tgbm::log::log(LEVEL, FMT __VA_OPT__(, ) __VA_ARGS__); \
  } while (0)

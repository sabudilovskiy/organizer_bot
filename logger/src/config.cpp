#include "tgbm_replace/backend.hpp"
#include "tgbm_replace/config.hpp"

#include <atomic>

namespace tgbm::log {

namespace {
std::atomic<direct_sink_fn> global_direct_sink_fn{sink::per_thread_file_with_flush};

std::atomic<level> global_log_level{level::debug};
}  // namespace

void set_direct_sink_fn(direct_sink_fn fn) noexcept {
  global_direct_sink_fn.store(fn, std::memory_order::relaxed);
}

void set_log_level(level lvl) noexcept {
  global_log_level.store(lvl, std::memory_order::relaxed);
}

void disable_logs() noexcept {
  set_log_level(level::_off);
}

direct_sink_fn get_direct_sink_fn() noexcept {
  return global_direct_sink_fn.load(std::memory_order::relaxed);
}

level get_log_level() noexcept {
  return global_log_level.load(std::memory_order::relaxed);
}

}  // namespace tgbm::log

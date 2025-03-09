#include "stop_handler.hpp"
#include <csignal>

namespace bot {

namespace {
std::atomic_bool need{};

void stop_handler(int signal) {
  need.store(true, std::memory_order::relaxed);
}

}  // namespace

void set_handler() {
  std::signal(SIGINT, stop_handler);
  std::signal(SIGTERM, stop_handler);
}

bool is_need_stop() {
  return need.load(std::memory_order::relaxed);
}

void need_stop() {
  need.store(true, std::memory_order::relaxed);
}

}  // namespace bot

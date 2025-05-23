#pragma once

#include <chrono>
#include <ctime>
#include <array>
#include <string_view>

#include "tgbm_replace/level.hpp"

namespace tgbm::log {

using ts_t = std::chrono::system_clock::time_point;

// Called with a fully-formatted log line.
// The function is responsible for writing the message to the final destination.
// If applicable, it should also update the byte counter for auto-flushing.
using direct_sink_fn = void (*)(level lvl, std::string_view str) noexcept;

inline std::string_view ts_str_now() {
  std::time_t t = std::time(nullptr);
  if (t == -1)
    std::terminate();
  std::tm tm;

#if defined(_MSC_VER)  // Microsoft Visual C++
  if (localtime_s(&tm, &t) != 0) {
    std::terminate();
  }
#else  // POSIX systems
  if (localtime_r(&t, &tm) == nullptr) {
    std::terminate();
  }
#endif

  thread_local std::array<char, 100> arr;
  std::string_view s(arr.data(), ::strftime(arr.data(), arr.size(), "%c %Z", &tm));
  if (s.empty())
    std::terminate();
  return s;
}

}  // namespace tgbm::log

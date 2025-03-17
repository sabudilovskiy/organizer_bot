#pragma once

#include <chrono>
#include <string>

#include "time/zone.hpp"

namespace bot {

struct ts_utc_t {
  using native_t = std::chrono::sys_time<std::chrono::nanoseconds>;

  ts_utc_t(native_t time = {}) noexcept;

  static ts_utc_t now() noexcept;
  static ts_utc_t never() noexcept;

  static ts_utc_t parse(std::string_view str);
  static bool parse(std::string_view str, ts_utc_t& out) noexcept;

  static ts_utc_t parse(std::string_view str, time_zone tz);
  static bool parse(std::string_view str, time_zone tz, ts_utc_t& out) noexcept;

  native_t native() const noexcept;

  std::string serialize() const;

  ts_utc_t operator+(std::chrono::nanoseconds ns) const noexcept;
  ts_utc_t operator-(std::chrono::nanoseconds ns) const noexcept;

  ts_utc_t operator+=(std::chrono::nanoseconds ns) noexcept;
  ts_utc_t operator-=(std::chrono::nanoseconds ns) noexcept;

  std::chrono::nanoseconds operator-(const ts_utc_t& rhs) const noexcept;

  auto operator<=>(const ts_utc_t& rhs) const = default;

 private:
  native_t time_;
};

}  // namespace bot

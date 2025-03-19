#pragma once

#include "time/ts.hpp"

namespace bot {

struct ts_zoned_t {
  using native_t = std::chrono::zoned_time<std::chrono::nanoseconds>;
  using local_t = std::chrono::local_time<std::chrono::nanoseconds>;

  ts_zoned_t(ts_utc_t ts, time_zone tz) noexcept;
  ts_zoned_t(local_t time, time_zone tz) noexcept;
  ts_zoned_t(native_t time) noexcept;

  ts_utc_t to_utc() const noexcept;

  static ts_zoned_t now(time_zone tz) noexcept;
  static ts_zoned_t never(time_zone tz) noexcept;

  native_t native() const noexcept;
  local_t local() const noexcept;
  time_zone zone() const noexcept;

  std::string serialize() const;

  ts_zoned_t operator+(std::chrono::nanoseconds ns) const noexcept;
  ts_zoned_t operator-(std::chrono::nanoseconds ns) const noexcept;

  ts_zoned_t operator+=(std::chrono::nanoseconds ns) noexcept;
  ts_zoned_t operator-=(std::chrono::nanoseconds ns) noexcept;

  std::chrono::nanoseconds operator-(ts_zoned_t ts) const noexcept;

  std::strong_ordering operator<=>(const ts_zoned_t& rhs) const noexcept;

 private:
  native_t time_;
  time_zone tz_;
};

}  // namespace bot

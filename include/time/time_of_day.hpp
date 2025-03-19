#pragma once

#include <cstdint>

#include "time/date.hpp"
#include "time/ts.hpp"
#include "time/ts_zoned.hpp"
#include "traits.hpp"

namespace bot {

struct time_of_day {
  time_of_day(std::int64_t hours = 0, std::int64_t minutes = 0) noexcept;

  std::int64_t hour() const noexcept;
  std::int64_t minutes() const noexcept;

  static time_of_day parse(std::string_view str);
  static bool parse(std::string_view str, time_of_day& out) noexcept;

  std::string serialize() const;

  bool is_valid() const noexcept;
  static bool is_valid(std::int64_t hour, std::int64_t minute) noexcept;

  static time_of_day from_ts(ts_utc_t ts) noexcept;
  static time_of_day from_ts(ts_zoned_t ts) noexcept;

  ts_utc_t to_ts(date date) const noexcept;
  ts_zoned_t to_ts_zoned(date date, time_zone tz) const noexcept;

  time_of_day operator+(std::chrono::minutes duration) const noexcept;
  std::chrono::minutes operator-(const time_of_day& rhs) const noexcept;

  auto operator<=>(const time_of_day& rhs) const = default;

 private:
  std::int64_t hour_;     // 0-23
  std::int64_t minutes_;  // 0-59
};

}  // namespace bot

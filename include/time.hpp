#pragma once

#include <chrono>

#include "error_policy.hpp"
#include "optional.hpp"

namespace bot {

using ts_t = std::chrono::local_time<std::chrono::nanoseconds>;

ts_t now();
ts_t to_zoned_time(const ts_t& ts);

std::string to_string(const ts_t& ts);

ts_t parse_ts(const std::string& str);
ts_t parse_short_ts(const std::string& str);

optional<ts_t> parse_ts(const std::string& str, use_optional) noexcept;
optional<ts_t> parse_short_ts(const std::string& str, use_optional) noexcept;

using weekday = std::chrono::weekday;

optional<weekday> parse_weekday(std::string_view str, use_optional) noexcept;
weekday parse_weekday(std::string_view str);

std::string_view to_string(weekday w) noexcept;

enum struct schedule_frequence {
  one_time,
  weekly,
  biweekly,
};

struct time_of_day {
  std::int64_t hour;    // 0-23
  std::int64_t minute;  // 0-59

  bool is_valid() const noexcept {
    return 0 <= hour && hour <= 23 && 0 <= minute && minute <= 59;
  }

  static bool is_valid(std::int64_t hour, std::int64_t minute) noexcept {
    return 0 <= hour && hour <= 23 && 0 <= minute && minute <= 59;
  }
};

struct schedule_unit {
  weekday weekday;
  time_of_day time;
  schedule_frequence frequence;
  ts_t start_date;
};

std::string_view human_frequence(schedule_frequence freq) noexcept;

}  // namespace bot

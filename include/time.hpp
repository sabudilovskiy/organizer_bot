#pragma once

#include <chrono>
namespace bot {

using ts_t = std::chrono::local_time<std::chrono::nanoseconds>;

ts_t now();
ts_t to_zoned_time(const ts_t& ts);

std::string to_string(const ts_t& ts);
ts_t parse_ts(const std::string& str);

using weekday = std::chrono::weekday;
using hours = std::chrono::hours;
using minutes = std::chrono::minutes;

std::string_view to_string(weekday w) noexcept;

enum struct shedule_frequence {
  one_time,
  weekly,
  biweekly,
};

struct time_of_day {
  int hour;    // 0-23
  int minute;  // 0-59
};

struct shedule_unit {
  weekday weekday;
  time_of_day time;
  shedule_frequence frequence;
  ts_t start_date;
};

}  // namespace bot

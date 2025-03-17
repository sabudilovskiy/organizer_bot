#include "time/schedule_unit.hpp"

#include <tgbm/utils/macro.hpp>

namespace bot {

state_today schedule_unit::get_state_today(ts_zoned_t now) const noexcept {
  auto today = date(now);

  if (weekday{today} != wd) {
    return state_today::other_day;
  }

  switch (frequence) {
    case schedule_frequence::one_time:
      if (active_from != today)
        return state_today::other_day;
      break;

    case schedule_frequence::weekly:
      if (active_from > today)
        return state_today::other_day;
      break;

    case schedule_frequence::biweekly:
      if (active_from > today || (today - active_from).count() % 14 != 0) {
        return state_today::other_day;
      }
      break;
  }

  auto now_time = time_of_day::from_ts(now);
  if (now_time > time) {
    return state_today::past;
  }

  return state_today::upcoming;
}

ts_zoned_t schedule_unit::next_occurence(ts_zoned_t start) const noexcept {
  date active_from(start);

  switch (frequence) {
    case schedule_frequence::weekly: {
      auto next_date = active_from;
      while (weekday{next_date} != wd) {
        next_date += std::chrono::days(1);
      }
      return time.to_ts_zoned(next_date, start.zone());
    }

    case schedule_frequence::biweekly: {
      auto next_date = active_from;
      while (weekday{next_date} != wd) {
        next_date += std::chrono::days(1);
      }
      return time.to_ts_zoned(next_date + std::chrono::weeks(1), start.zone());
    }

    case schedule_frequence::one_time: {
      ts_zoned_t scheduled_time = time.to_ts_zoned(active_from, start.zone());
      return (start > scheduled_time) ? start : scheduled_time;
    }
  }
  tgbm::unreachable();
}
}  // namespace bot

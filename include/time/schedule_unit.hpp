#pragma once

#include "time/date.hpp"
#include "time/schedule_frequence.hpp"
#include "time/time_of_day.hpp"
#include "time/weekday.hpp"

namespace bot {

enum struct state_today {
  past,
  active,
  upcoming,
  other_day,
};

struct schedule_unit {
  weekday wd;
  time_of_day time;
  schedule_frequence frequence;
  date active_from;

  state_today get_state_today(ts_zoned_t now) const noexcept;

  ts_zoned_t next_occurence(ts_zoned_t start) const noexcept;
};
}  // namespace bot

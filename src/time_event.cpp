#include "time_event.hpp"

#include "time/date.hpp"

namespace bot {

reminder_all_calls_meta_t& time_event::reminder_all_calls_meta() {
  return std::get<reminder_all_calls_meta_t>(meta);
}

const reminder_all_calls_meta_t& time_event::reminder_all_calls_meta() const {
  return std::get<reminder_all_calls_meta_t>(meta);
}

reminder_call_meta_t& time_event::reminder_call_meta() {
  return std::get<reminder_call_meta_t>(meta);
}

const reminder_call_meta_t& time_event::reminder_call_meta() const {
  return std::get<reminder_call_meta_t>(meta);
}

time_event_type time_event::type() const {
  return (time_event_type)meta.index();
}

ts_zoned_t reminder_all_calls_meta_t::next_occurence(ts_zoned_t start) const {
  if (time_points.empty()) {
    return ts_zoned_t::never(start.zone());
  }

  auto day = date(start);
  time_of_day start_time = time_of_day::from_ts(start);

  auto it = time_points.lower_bound(start_time);

  if (it != time_points.end()) {
    return it->to_ts_zoned(day, start.zone());
  }

  day += std::chrono::days(1);

  return time_points.begin()->to_ts_zoned(day, start.zone());
}
}  // namespace bot

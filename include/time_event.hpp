#pragma once

#include <io_event.hpp>

namespace bot {

enum struct time_event_type {
  reminder_all_calls,
  reminder_call,
};

struct reminder_all_calls_meta_t {
  std::vector<time_of_day> time_points;
};

struct reminder_call_meta_t {
  int64_t call_id;
};

using time_event_meta = std::variant<reminder_all_calls_meta_t, reminder_call_meta_t>;

struct time_event {
  int64_t time_event_id;
  tgbm::api::optional<int64_t> user_id;
  ts_t next_occurence;
  time_event_meta meta;
  bool consumed;

  static constexpr std::string_view db_name = "time_events";
  using meta_type = time_event_type;

  reminder_all_calls_meta_t& reminder_all_calls_meta();
  const reminder_all_calls_meta_t& reminder_all_calls_meta() const;

  reminder_call_meta_t& reminder_call_meta();
  const reminder_call_meta_t& reminder_call_meta() const;

  time_event_type type() const;
};

}  // namespace bot

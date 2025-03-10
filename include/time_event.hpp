#pragma once

#include <io_event.hpp>

namespace bot {

enum struct time_event_type {
  reminder_all_calls,
  reminder_call,
};

struct reminder_all_calls_meta_t {
  int64_t user_id;
};

struct reminder_call_meta_t {
  int64_t user_id;
  int64_t call_id;
};

using time_event_meta = std::variant<reminder_all_calls_meta_t, reminder_call_meta_t>;

static_assert(magic_enum::enum_names<time_event_type>() == meta_names_v<time_event_meta>);

struct time_event {
  int64_t time_event_id;
  ts_t next_occurence;
  time_event_meta meta;
  bool consumed;

  static constexpr std::string_view db_name = "time_events";

  reminder_all_calls_meta_t& reminder_all_calls_meta();
  const reminder_all_calls_meta_t& reminder_all_calls_meta() const;

  reminder_call_meta_t& reminder_call_meta();
  const reminder_call_meta_t& reminder_call_meta() const;

  time_event_type type();
};

template <>
struct json_reader<time_event_meta> : json_reader_meta<time_event_meta, time_event_type> {};

template <>
struct json_writer<time_event_meta> : json_writer_meta<time_event_meta, time_event_type> {};

}  // namespace bot

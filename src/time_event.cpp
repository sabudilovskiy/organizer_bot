#include "time_event.hpp"

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

time_event_type time_event::type() {
  return (time_event_type)meta.index();
}

}  // namespace bot

#include "time_event_dispatcher.hpp"

namespace bot {

namespace {

consumer_t handle_all_calls(OrganizerDB& db, const tgbm::api::telegram& api, time_event event) {
  auto calls = db.getCalls(event.reminder_all_calls_meta().user_id);
}

}  // namespace

void time_event_dispatcher::push(time_event event) {
  event.time_event_id = db.addTimeEvent(event);
  queue.push(std::move(event));
}

void time_event_dispatcher::save() {
  db.consumeEvents(consumed_events);
  consumed_events.clear();
  TGBM_LOG_INFO("Saved {} consumed time_events", consumed_events.size());
}

void time_event_dispatcher::load() {
  auto events = db.getTimeEvents(now() + std::chrono::hours(1));
  for (auto& e : events) {
    queue.push(std::move(e));
  }
}

consumer_t time_event_dispatcher::handle(time_event event) {
  auto id = event.time_event_id;
  switch (event.type()) {
    case time_event_type::reminder_all_calls:
    case time_event_type::reminder_call:
      break;
  }
  consumed_events.push_back(id);
  co_return;
}

}  // namespace bot

#include "time_event_dispatcher.hpp"

#include "time_event_handlers.hpp"
#include "user_context.hpp"

namespace bot {

std::int64_t time_event_dispatcher::push(time_event event) {
  auto id = db.addTimeEvent(event);
  event.time_event_id = id;
  queue.push(std::move(event));
  return id;
}

void time_event_dispatcher::save() {
  db.consumeTimeEvents(consumed_events);
  auto size = consumed_events.size();
  consumed_events.clear();
  if (size != 0)
    TGBM_LOG_INFO("Saved consumed {} time_events", size);
  else {
    TGBM_LOG_DEBUG("Saved consumed {} time_events", size);
  }
}

void time_event_dispatcher::load() {
  auto events = db.getTimeEvents(ts_utc_t::now() + std::chrono::hours(1));
  for (auto& e : events) {
    queue.push(std::move(e));
  }
}

consumer_t time_event_dispatcher::handle(time_event event) {
  auto id = event.time_event_id;
  switch (event.type()) {
    case time_event_type::reminder_all_calls:
      AWAIT_ALL(handle_all_calls(db, api, *this, std::move(event)));
      break;
    case time_event_type::reminder_call:
      break;
  }
  consumed_events.emplace(id);
  co_return;
}

const time_event* time_event_dispatcher::top() {
  while (!queue.empty() && consumed_events.contains(queue.top().time_event_id)) {
    queue.pop();
  }
  return !queue.empty() ? &queue.top() : nullptr;
}

ts_utc_t time_event_dispatcher::next_occurenece() {
  auto t = top();
  return t ? t->next_occurence : ts_utc_t::never();
}

consumer_t time_event_dispatcher::execute() {
  auto now_v = ts_utc_t::now();
  while (top() && now_v - top()->next_occurence >= std::chrono::seconds(1)) {
    auto e = *top();
    queue.pop();
    auto id = e.time_event_id;
    consume(id);
    AWAIT_ALL(handle(std::move(e)));
  }
  auto l = top();
  if (!l) {
    TGBM_LOG_DEBUG("0 time_events to execute");
  } else {
    TGBM_LOG_DEBUG("{} time_events to execute. Near: {}", queue.size(),
                   l->next_occurence);
  }
}

void time_event_dispatcher::consume(std::int64_t event_id) {
  consumed_events.emplace(event_id);
}

}  // namespace bot

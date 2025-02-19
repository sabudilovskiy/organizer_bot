#include "event_broker.hpp"
#include "dialogs.hpp"

namespace bot {

EventBroker::EventBroker(const tgbm::api::telegram& api, Database& db) noexcept : api(api), db(db) {
}

dd::task<void> EventBroker::add(ts_t ts, std::int64_t user_id, EventType type, json_value meta) {
  auto& user_events = events[user_id];
  user_events.emplace_back(Event{
      .event_id = db.addEvent(ts, user_id, type, meta),
      .ts = ts,
      .user_id = user_id,
      .meta = std::move(meta),
  });
  auto& user_consumer = consumers[user_id];
  if (user_consumer.empty()) {
    user_consumer = start_dialog(UserCtx{
        .db = db,
        .api = api,
        .events = user_events,
        .user_id = user_id,
    });
  }
  (void)co_await user_consumer.begin();

  if (user_consumer.empty()) {
    consumers.erase(user_id);
  }
}

void EventBroker::load() {
  auto db_events = db.getEvents();
  auto fn = [](const Event& l, const Event& r) { return l.user_id == r.user_id; };
  for (auto&& group : db_events | std::ranges::views::chunk_by(fn)) {
    auto user_id = group.front().user_id;
    std::move(group.begin(), group.end(), std::back_inserter(events[user_id]));
  }
}

std::vector<Event>& EventBroker::get_events(std::int64_t user_id) {
  return events[user_id];
}

}  // namespace bot

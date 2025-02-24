#include "event_broker.hpp"

#include "dialogs.hpp"
#include "formatters/aggregate.hpp"
#include "tgbm_replace/logger.hpp"

namespace bot {

namespace {
dd::task<tgbm::api::optional<EventRawData>> generate_event(tgbm::api::Update u, Database& db,
                                                           const tgbm::api::telegram& api) {
  if (auto* msg = u.get_message(); msg && msg->from && msg->chat && msg->chat->type == "private") {
    db.fetchUser(RequestUser{.user_id = msg->from->id, .chat_id = msg->chat->id});
    json_value meta = json_value::object();
    meta["text"] = msg->text;
    co_return EventRawData{
        .ts = now(),
        .user_id = msg->from->id,
        .type = EventType::message,
        .meta = std::move(meta),
    };
  } else if (auto* query = u.get_callback_query(); query && query->from) {
    json_value meta = json_value::object();
    meta["id"] = query->id;
    meta["data"] = query->data;

    co_return EventRawData{
        .ts = now(),
        .user_id = query->from->id,
        .type = EventType::callback_query,
        .meta = std::move(meta),
    };
  }

  co_return std::nullopt;
}
}  // namespace

EventBroker::EventBroker(const tgbm::api::telegram& api, Database& db) noexcept : api_(api), db_(db) {
}

dd::task<void> EventBroker::process_update(tgbm::api::Update update) {
  auto _event = co_await generate_event(std::move(update), db_, api_);
  if (!_event) {
    co_return;
  }
  auto event = std::move(*_event);
  TGBM_LOG_EVENT("generated event: {}", event);

  auto& user_events = events_[event.user_id];
  auto event_id = db_.addEvent(event);
  user_events.emplace_back(Event{
      .event_id = event_id,
      .ts = event.ts,
      .user_id = event.user_id,
      .type = event.type,
      .meta = std::move(event.meta),
  });
  auto& user_consumer = consumers_[event.user_id];
  if (user_consumer.empty()) {
    user_consumer = start_dialog(UserCtx{
        .db = db_,
        .api = api_,
        .events = user_events,
        .user_id = event.user_id,
    });
  }
  (void)co_await user_consumer.begin();

  if (user_consumer.empty()) {
    user_consumer = start_dialog(UserCtx{
        .db = db_,
        .api = api_,
        .events = user_events,
        .user_id = event.user_id,
    });
  }

  (void)co_await user_consumer.begin();
}

void EventBroker::load() {
  auto db_events = db_.getEvents();
  auto fn = [](const Event& l, const Event& r) { return l.user_id == r.user_id; };
  for (auto&& group : db_events | std::ranges::views::chunk_by(fn)) {
    auto user_id = group.front().user_id;
    std::move(group.begin(), group.end(), std::back_inserter(events_[user_id]));
  }
}

std::vector<Event>& EventBroker::get_events(std::int64_t user_id) {
  return events_[user_id];
}

void EventBroker::save() {
  std::vector<int64_t> consumed_events;

  for (auto& [_, events] : events_) {
    for (auto& e : events) {
      if (e.consumed) {
        consumed_events.emplace_back(e.event_id);
      }
    }
  }

  for (auto& [_, events] : events_) {
    events.erase(std::remove_if(events.begin(), events.end(), [](Event& e) { return e.consumed; }),
                 events.end());
  }
  if (!consumed_events.empty()) {
    db_.consumeEvents(consumed_events);
  }
}
}  // namespace bot

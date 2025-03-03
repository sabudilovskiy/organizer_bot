#include "event_broker.hpp"

#include <ranges>

#include "dialogs.hpp"
#include "event_utils.hpp"
#include "formatters/aggregate.hpp"
#include "tgbm_replace/logger.hpp"

namespace bot {

namespace {
dd::task<tgbm::api::optional<Event>> generate_event(tgbm::api::Update u, Database& db,
                                                    const tgbm::api::telegram& api) {
  if (auto* msg = u.get_message(); msg && msg->from && msg->chat && msg->chat->type == "private") {
    db.fetchUser(RequestUser{.user_id = msg->from->id, .chat_id = msg->chat->id});
    EventMeta meta;
    if (msg->text->starts_with("/")) {
      meta = CommandMeta{.text = msg->text->substr(1)};
    } else {
      meta = MessageMeta{.text = *msg->text};
    }
    co_return Event{
        .user_id = msg->from->id,
        .ts = now(),
        .meta = std::move(meta),
    };
  } else if (auto* query = u.get_callback_query(); query && query->from && query->data) {
    (void)co_await api
        .answerCallbackQuery({
            .callback_query_id = query->id,
            .cache_time = 3,
        })
        .wait();
    co_return Event{
        .user_id = query->from->id,
        .ts = now(),
        .meta =
            CBQueryMeta{
                .id = query->id,
                .data = *query->data,
            },
    };
  }

  co_return std::nullopt;
}
}  // namespace

EventBroker::EventBroker(const tgbm::api::telegram& api, Database& db) noexcept : api_(api), db_(db) {
}

dd::task<void> EventBroker::process_update(tgbm::api::Update update) {
  auto event = co_await generate_event(std::move(update), db_, api_);
  if (!event) {
    co_return;
  }
  co_await process_event(std::move(*event));
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

dd::task<void> EventBroker::safe_process_update(tgbm::api::Update update) noexcept {
  auto update_id = update.update_id;
  auto t = process_update(std::move(update));
  try {
    co_await t;
  } catch (std::exception& exc) {
    TGBM_LOG_ERROR("Fail processing update {}. Happens: {}", update_id, exc.what());
  }
}

void EventBroker::add_deferred_event(Event event) {
  deferred_events_.emplace_back(std::move(event));
}

dd::task<void> EventBroker::process_event(Event event) {
  TGBM_LOG_DEBUG("Processed: {}", event);

  auto& user_events = events_[event.user_id];
  event.event_id = db_.addEvent(event);
  user_events.emplace_back(std::move(event));
  auto& user_consumer = consumers_[event.user_id];
  if (events::should_main_menu(user_events) || user_consumer.empty()) {
    auto user = db_.fetchUser({.user_id = event.user_id});

    user_consumer = main_menu(
        Context{
            .db = db_,
            .api = api_,
            .event_broker = *this,
            .events = user_events,
            .user_id = event.user_id,
        },
        std::move(user));
  }
  (void)co_await user_consumer.begin();

  if (user_consumer.empty()) {
    user_consumer = main_menu(Context{
        .db = db_,
        .api = api_,
        .event_broker = *this,
        .events = user_events,
        .user_id = event.user_id,
    });
  }

  (void)co_await user_consumer.begin();
}

dd::task<void> EventBroker::process_defferred_events() {
  for (auto& e : deferred_events_) {
    co_await process_event(std::move(e));
  }
}

}  // namespace bot

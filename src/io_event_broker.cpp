#include "io_event_broker.hpp"

#include <ranges>

#include "dialogs.hpp"
#include "io_event_utils.hpp"
#include "formatters/aggregate.hpp"
#include "tgbm_replace/logger.hpp"

namespace bot {

namespace {
dd::task<tgbm::api::optional<io_event>> generate_event(tgbm::api::Update u, OrganizerDB& db,
                                                       const tgbm::api::telegram& api) {
  if (auto* msg = u.get_message(); msg && msg->from && msg->chat && msg->chat->type == "private") {
    db.fetchUser(RequestUser{.user_id = msg->from->id, .chat_id = msg->chat->id});
    io_event_meta meta;
    if (msg->text->starts_with("/")) {
      meta = command_meta_t{.text = msg->text->substr(1)};
    } else {
      meta = message_meta_t{.text = *msg->text};
    }
    co_return io_event{
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
    co_return io_event{
        .user_id = query->from->id,
        .ts = now(),
        .meta =
            cb_query_meta_t{
                .id = query->id,
                .data = *query->data,
            },
    };
  }

  co_return std::nullopt;
}
}  // namespace

io_event_broker::io_event_broker(const tgbm::api::telegram& api, OrganizerDB& db) noexcept
    : api_(api), db_(db) {
}

dd::task<void> io_event_broker::process_update(tgbm::api::Update update) {
  auto event = co_await generate_event(std::move(update), db_, api_);
  if (!event) {
    co_return;
  }
  co_await process_event(std::move(*event));
}

void io_event_broker::load() {
  auto db_events = db_.getEvents();
  auto fn = [](const io_event& l, const io_event& r) { return l.user_id == r.user_id; };
  for (auto&& group : db_events | std::ranges::views::chunk_by(fn)) {
    auto user_id = group.front().user_id;
    std::move(group.begin(), group.end(), std::back_inserter(events_[user_id]));
  }
}

std::vector<io_event>& io_event_broker::get_events(std::int64_t user_id) {
  return events_[user_id];
}

void io_event_broker::save() {
  std::vector<int64_t> consumed_events;

  for (auto& [_, events] : events_) {
    for (auto& e : events) {
      if (e.consumed) {
        consumed_events.emplace_back(e.io_event_id);
      }
    }
  }

  for (auto& [_, events] : events_) {
    events.erase(std::remove_if(events.begin(), events.end(), [](io_event& e) { return e.consumed; }),
                 events.end());
  }
  if (!consumed_events.empty()) {
    db_.consumeEvents(consumed_events);
  }
  TGBM_LOG_INFO("Saved consumed {} io_events", consumed_events.size());
}

dd::task<void> io_event_broker::safe_process_update(tgbm::api::Update update) noexcept {
  auto update_id = update.update_id;
  auto t = process_update(std::move(update));
  try {
    co_await t;
  } catch (std::exception& exc) {
    TGBM_LOG_ERROR("Fail processing update {}. Happens: {}", update_id, exc.what());
  }
}

void io_event_broker::add_deferred_event(io_event event) {
  deferred_events_.emplace_back(std::move(event));
}

dd::task<void> io_event_broker::process_event(io_event event) {
  TGBM_LOG_DEBUG("Processed: {}", event);

  auto& user_events = events_[event.user_id];
  event.io_event_id = db_.addEvent(event);
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

dd::task<void> io_event_broker::process_defferred_events() {
  for (auto& e : deferred_events_) {
    co_await process_event(std::move(e));
  }
}

dd::task<void> io_event_broker::process_old_events() {
  for (auto& [user_id, user_events] : events_) {
    auto& user_consumer = consumers_[user_id];
    if (events::should_main_menu(user_events) || user_consumer.empty()) {
      auto user = db_.fetchUser({.user_id = user_id});

      user_consumer = main_menu(
          Context{
              .db = db_,
              .api = api_,
              .event_broker = *this,
              .events = user_events,
              .user_id = user_id,
          },
          std::move(user));
    }
    (void)co_await user_consumer.begin();
  }
}
}  // namespace bot

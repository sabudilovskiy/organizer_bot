#pragma once

#include <kelcoro/channel.hpp>
#include <kelcoro/generator.hpp>
#include <kelcoro/task.hpp>
#include <tgbm/api/telegram.hpp>

#include "database.hpp"
#include "types.hpp"

namespace bot {

using consumer_t = dd::channel<dd::nothing_t>;

struct EventBroker {
  EventBroker(const tgbm::api::telegram& api, Database& db) noexcept;

  void add_deferred_event(Event event);

  // log and ignore or exceptions
  dd::task<void> safe_process_update(tgbm::api::Update update) noexcept;

  dd::task<void> process_update(tgbm::api::Update update);

  dd::task<void> process_event(Event event);

  dd::task<void> process_defferred_events();

  std::vector<Event>& get_events(std::int64_t user_id);

  void load();

  void save();

 private:
  Database& db_;
  const tgbm::api::telegram& api_;
  std::map<std::int64_t, std::vector<Event>> events_;
  std::unordered_map<std::int64_t, consumer_t> consumers_;
  std::vector<Event> deferred_events_;
};

}  // namespace bot

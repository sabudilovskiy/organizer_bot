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

  dd::task<void> process_update(tgbm::api::Update update);

  std::vector<Event>& get_events(std::int64_t user_id);

  void load();

  void save();

 private:
  Database& db_;
  const tgbm::api::telegram& api_;
  std::map<std::int64_t, std::vector<Event>> events_;
  std::unordered_map<std::int64_t, consumer_t> consumers_;
};

}  // namespace bot

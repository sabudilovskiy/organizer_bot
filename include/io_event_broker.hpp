#pragma once

#include <kelcoro/channel.hpp>
#include <kelcoro/generator.hpp>
#include <kelcoro/task.hpp>
#include <tgbm/api/telegram.hpp>

#include "consumer.hpp"
#include "organizer_db.hpp"
#include "types.hpp"

namespace bot {

struct io_event_broker {
  io_event_broker(const tgbm::api::telegram& api, OrganizerDB& db) noexcept;

  void add_deferred_event(io_event event);

  dd::task<void> safe_process_update(tgbm::api::Update update) noexcept;

  dd::task<void> process_update(tgbm::api::Update update);

  dd::task<void> process_event(io_event event);

  dd::task<void> process_defferred_events();

  std::vector<io_event>& get_events(std::int64_t user_id);

  void load();

  void save();

 private:
  OrganizerDB& db_;
  const tgbm::api::telegram& api_;
  std::map<std::int64_t, std::vector<io_event>> events_;
  std::unordered_map<std::int64_t, consumer_t> consumers_;
  std::vector<io_event> deferred_events_;
};

}  // namespace bot

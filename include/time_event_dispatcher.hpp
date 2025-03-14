#pragma once

#include <queue>
#include <unordered_set>

#include "consumer.hpp"
#include "macro.hpp"
#include "organizer_db.hpp"
#include "tgbm/api/telegram.hpp"
#include "time.hpp"
#include "time_event.hpp"

namespace bot {

struct time_event_dispatcher {
  using next_occur_comp = decltype([](const time_event& lhs, const time_event& rhs) {
    return lhs.next_occurence > rhs.next_occurence;
  });

  const time_event* top();

  ts_t next_occurenece();

  void consume(std::int64_t event_id);

  time_event_dispatcher(const tgbm::api::telegram& api, OrganizerDB& db) noexcept
      : api(api), db(db) {
  }

  void load();

  consumer_t handle(time_event event);

  consumer_t execute();

  void push(time_event event);

  void save();

 private:
  const tgbm::api::telegram& api;
  OrganizerDB& db;
  std::priority_queue<time_event, std::vector<time_event>, next_occur_comp> queue;
  std::unordered_set<int64_t> consumed_events;
};

}  // namespace bot

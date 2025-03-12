#pragma once

#include <queue>

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

  ts_t next_occurenece() {
    if (queue.empty()) {
      return std::chrono::local_time<std::chrono::nanoseconds>::max();
    }
    return queue.top().next_occurence;
  }

  time_event_dispatcher(const tgbm::api::telegram& api, OrganizerDB& db) noexcept
      : api(api), db(db) {
  }

  void load();

  consumer_t handle(time_event event);

  consumer_t execute() {
    auto now_v = now();
    while (!queue.empty() &&
           queue.top().next_occurence - now_v >= std::chrono::seconds(1)) {
      auto e = queue.top();
      queue.pop();
      auto id = e.time_event_id;
      AWAIT_ALL(handle(std::move(e)));
      consumed_events.push_back(id);
    }
  }

  void push(time_event event);

  void save();

 private:
  const tgbm::api::telegram& api;
  OrganizerDB& db;
  std::priority_queue<time_event, std::vector<time_event>, next_occur_comp> queue;
  std::vector<int64_t> consumed_events;
};

}  // namespace bot

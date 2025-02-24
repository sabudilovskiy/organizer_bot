#pragma once
#include <vector>

namespace tgbm::api {
struct telegram;
}

namespace bot {

struct Database;
struct Event;

struct UserCtx {
  Database& db;
  const tgbm::api::telegram& api;
  std::vector<Event>& events;
  std::int64_t user_id;
};

}  // namespace bot

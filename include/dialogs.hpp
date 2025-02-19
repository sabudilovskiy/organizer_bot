#pragma once
#include "event_broker.hpp"
#include "tgbm/api/telegram.hpp"

namespace bot {

struct UserCtx {
  Database& db;
  const tgbm::api::telegram& api;
  std::vector<Event>& events;
  std::int64_t user_id;
};

consumer_t start_dialog(UserCtx ctx);

consumer_t create_task(UserCtx ctx);

consumer_t read_text(UserCtx ctx, std::string& text);

}  // namespace bot

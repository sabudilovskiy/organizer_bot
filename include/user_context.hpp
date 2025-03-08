#pragma once

#include <vector>

#include "consumer.hpp"

namespace tgbm::api {
struct telegram;
}

namespace bot {

struct Database;
struct io_event;
struct User;
struct io_event_broker;

struct Context {
  Database& db;
  const tgbm::api::telegram& api;
  io_event_broker& event_broker;
  std::vector<io_event>& events;
  std::int64_t user_id;
};

struct ContextWithUser {
  Database& db;
  const tgbm::api::telegram& api;
  io_event_broker& event_broker;

  std::vector<io_event>& events;
  std::int64_t user_id;
  User& user;

  ContextWithUser(Context& ctx, User& user) noexcept
      : db(ctx.db),
        api(ctx.api),
        event_broker(ctx.event_broker),
        events(ctx.events),
        user_id(ctx.user_id),
        user(user) {
  }

  [[nodiscard]] action_t send_text(std::string text);
  [[nodiscard]] consumer_t read_text(std::string text, std::string& out);
  [[nodiscard]] consumer_t delete_message(int64_t id);

  void to_main_menu();
  void set_need_new_message();
};

}  // namespace bot

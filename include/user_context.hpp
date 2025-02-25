#pragma once

#include <kelcoro/generator.hpp>
#include <kelcoro/channel.hpp>
#include <vector>

namespace tgbm::api {
struct telegram;
}

namespace bot {

using consumer_t = dd::channel<dd::nothing_t>;

struct Database;
struct Event;
struct User;

struct Context {
  Database& db;
  const tgbm::api::telegram& api;
  std::vector<Event>& events;
  std::int64_t user_id;
};

struct ContextWithUser {
  Database& db;
  const tgbm::api::telegram& api;
  std::vector<Event>& events;
  std::int64_t user_id;
  User& user;

  ContextWithUser(Context& ctx, User& user) noexcept
      : db(ctx.db), api(ctx.api), events(ctx.events), user_id(ctx.user_id), user(user) {
  }

  consumer_t send_text(std::string text);
  consumer_t delete_message(int64_t id);
};

}  // namespace bot

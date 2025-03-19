#pragma once

#include <vector>

#include "consumer.hpp"
#include "time_event_dispatcher.hpp"

namespace tgbm::api {
struct telegram;
}

namespace bot {

struct OrganizerDB;
struct io_event;
struct User;
struct io_event_broker;

struct Context {
  OrganizerDB& db;
  const tgbm::api::telegram& api;
  io_event_broker& event_broker;
  time_event_dispatcher& time_event_dispatcher;
  std::vector<io_event>& events;
};

struct ContextWithUser {
  OrganizerDB& db;
  const tgbm::api::telegram& api;
  io_event_broker& io_event_broker;
  time_event_dispatcher& time_event_dispatcher;
  std::vector<io_event>& events;
  User& user;

  ContextWithUser(Context& ctx, User& user) noexcept
      : db(ctx.db),
        api(ctx.api),
        io_event_broker(ctx.event_broker),
        time_event_dispatcher(ctx.time_event_dispatcher),
        events(ctx.events),
        user(user) {
  }

  [[nodiscard]] action_t send_text(std::string text);
  [[nodiscard]] consumer_t read_text(std::string text, std::string& out);
  [[nodiscard]] consumer_t read_time(std::string text, time_of_day& out);
  [[nodiscard]] consumer_t read_positive_number(std::string text, std::int64_t& out);
  [[nodiscard]] consumer_t delete_message(int64_t id);

  void to_main_menu();
  void set_need_new_message();
};

}  // namespace bot

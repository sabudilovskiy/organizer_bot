#include "dialogs.hpp"

namespace bot {

void consume_first_event(UserCtx ctx) {
  for (auto& e : ctx.events) {
    if (!e.consumed && e.type == EventType::message) {
      e.consumed = true;
      return;
    }
  }
}

consumer_t read_text(UserCtx ctx, const User& user, std::string& text, std::string message) {
  auto try_read = [&]() {
    for (auto& e : ctx.events) {
      if (!e.consumed && e.type == EventType::message) {
        text = e.meta["text"].as<std::string>();
        e.consumed = true;
        return true;
      }
    }
    return false;
  };
  if (try_read())
    co_return;

  (void)co_await ctx.api.sendMessage(
      tgbm::api::send_message_request{.chat_id = user.chat_id, .text = message});
  for (;;) {
    co_yield {};
    if (try_read())
      co_return;
  }
}

consumer_t start_dialog(UserCtx ctx) {
  auto user = ctx.db.fetchUser(RequestUser{.user_id = ctx.user_id});
  auto& api = ctx.api;

  std::string name, date;

  consume_first_event(ctx);

  co_yield dd::elements_of(read_text(ctx, user, name, "Type your name: "));

  co_yield dd::elements_of(read_text(ctx, user, date, "Type your date: "));

  (void)co_await api.sendMessage(tgbm::api::send_message_request{
      .chat_id = user.chat_id, .text = fmt::format("End dialog. name: {}, date: {}", name, date)});
}

}  // namespace bot

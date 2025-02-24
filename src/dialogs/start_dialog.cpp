#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"

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

  (void)co_await ctx.api.sendMessage(tgbm::api::send_message_request{
      .chat_id = user.chat_id,
      .text = std::move(message),
  });
  for (;;) {
    co_yield {};
    if (try_read())
      co_return;
  }
}

enum struct Question { yes, no };

consumer_t start_dialog(UserCtx ctx) {
  TGBM_LOG("start_dialog: {}", ctx.user_id);
  auto user = ctx.db.fetchUser(RequestUser{.user_id = ctx.user_id});
  auto& api = ctx.api;

  std::string name, date;

  consume_first_event(ctx);

  Question res;
  // clang-format off
  auto menu = Menu<Question>{"Wtf?"}
    .add("yes", Question::yes)
    .add("no", Question::no);
  // clang-format on
  $await_all(menu.show(ctx, user, res));

  if (res == Question::yes) {
    (void)co_await ctx.api.sendMessage(tgbm::api::send_message_request{
        .chat_id = user.chat_id,
        .text = "You answer: Yes",
    });
  } else {
    (void)co_await ctx.api.sendMessage(tgbm::api::send_message_request{
        .chat_id = user.chat_id,
        .text = "You answer: No",
    });
  }
}

}  // namespace bot

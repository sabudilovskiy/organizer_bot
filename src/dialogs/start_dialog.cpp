#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {

void consume_first_event(Context ctx) {
  for (auto& e : ctx.events | events::only_messages | events::take(1)) {
    e.consumed = true;
  }
}

consumer_t read_text(Context ctx, const User& user, std::string& text, std::string message) {
  auto try_read = [&]() {
    for (auto& e : ctx.events | events::only_messages) {
      text = e.message_meta().text;
      e.consumed = true;
      return true;
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

consumer_t start_dialog(Context ctx) {
  TGBM_LOG("start_dialog: {}", ctx.user_id);
  auto user = ctx.db.fetchUser(RequestUser{.user_id = ctx.user_id});
  ContextWithUser ctx_user(ctx, user);

  std::string name, date;

  consume_first_event(ctx);

  Question res;
  // clang-format off
  auto menu = Menu<Question>{"Test menu"}
    .add("yes", Question::yes)
    .add("no", Question::no);
  // clang-format on
  $await_all(menu.show(ctx, user, res));

  if (res == Question::yes) {
    $await_all(ctx_user.send_text("Your answer : yes"));
  } else {
    $await_all(ctx_user.send_text("Your answer : no"));
  }
}

}  // namespace bot

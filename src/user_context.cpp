#include "user_context.hpp"

#include <tgbm/api/telegram.hpp>
#include <tgbm/api/types/all.hpp>

#include "database.hpp"
#include "io_event_broker.hpp"
#include "io_event_utils.hpp"
#include "macro.hpp"
#include "types.hpp"

namespace bot {

action_t ContextWithUser::send_text(std::string text) {
  (void)co_await api.sendMessage({
      .chat_id = user.chat_id,
      .text = std::move(text),
  });
  user.additional_messages++;
  db.updateUser(user);
}

consumer_t ContextWithUser::delete_message(int64_t id) {
  (void)co_await api.deleteMessage({
      .chat_id = user.chat_id,
      .message_id = id,
  });
}

void ContextWithUser::to_main_menu() {
  event_broker.add_deferred_event({
      .user_id = user_id,
      .ts = now(),
      .meta = command_meta_t{"start"},
  });
}

consumer_t ContextWithUser::read_text(std::string text, std::string& out) {
  co_await send_text(std::move(text));

  for (;;) {
    for (auto& e : this->events | events::only_messages | events::take(1)) {
      out = e.message_meta().text;
      e.consumed = true;
      co_return;
    }
    co_yield {};
  }
}

void ContextWithUser::set_need_new_message() {
  user.set_need_new_message();
  db.updateUser(user);
}
}  // namespace bot

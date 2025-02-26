#include "user_context.hpp"

#include <tgbm/api/telegram.hpp>
#include <tgbm/api/types/all.hpp>

#include "database.hpp"
#include "event_broker.hpp"
#include "types.hpp"

namespace bot {
consumer_t ContextWithUser::send_text(std::string text) {
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
      .meta = CommandMeta{"start"},
  });
}
}  // namespace bot

#include "user_context.hpp"

#include <tgbm/api/telegram.hpp>
#include <tgbm/api/types/all.hpp>

#include "types.hpp"

namespace bot {
consumer_t ContextWithUser::send_text(std::string text) const {
  (void)co_await api.sendMessage(tgbm::api::send_message_request{
      .chat_id = user.chat_id,
      .text = std::move(text),
  });
}

consumer_t ContextWithUser::delete_message(int64_t id) const {
  (void)co_await api.deleteMessage(tgbm::api::delete_message_request{
      .chat_id = user.chat_id,
      .message_id = id,
  });
}
}  // namespace bot

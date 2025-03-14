#include "user_context.hpp"

#include <tgbm/api/telegram.hpp>
#include <tgbm/api/types/all.hpp>

#include "database.hpp"
#include "io_event_broker.hpp"
#include "io_event_utils.hpp"
#include "macro.hpp"
#include "scn/scan.h"
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
  io_event_broker.add_deferred_event({
      .user_id = user.user_id,
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

consumer_t ContextWithUser::read_time(std::string text, time_of_day& out) {
  out = time_of_day{-1, -1};
  std::string input;
  AWAIT_ALL(read_text(std::move(text), input));
  auto result = scn::scan<int, int>(input, "{:d}:{:d}");
  bool failed = !result.has_value();
  if (!failed) {
    auto [h, m] = result->values();
    out = time_of_day{h, m};
  }
  while (!out.is_valid()) {
    AWAIT_ALL(
        read_text("⚠️ Ошибка ввода! Время должно быть в формате ЧЧ:ММ (например, 15:30)."
                  "🔄 Попробуйте ещё раз и введите время в правильном формате: ",
                  input));
    result = scn::scan<int, int>(input, "{:d}:{:d}");
    failed = !result.has_value();
    if (!failed) {
      auto [h, m] = result->values();
      out = time_of_day{h, m};
    }
  }
}

consumer_t ContextWithUser::read_positive_number(std::string text, std::int64_t& out) {
  std::string input;
  AWAIT_ALL(read_text(std::move(text), input));

  auto result = scn::scan<std::int64_t>(input, "{:d}");
  while (!result.has_value()) {
    AWAIT_ALL(read_text(
        R"(⚠️ Ошибка ввода! Ввод должен быть положительным числом.
🔄 Попробуйте ещё раз и введите длительность в правильном формате: )",
        input));
    result = scn::scan<std::int64_t>(input, "{:d}");
  }
  out = result->value();
}
}  // namespace bot

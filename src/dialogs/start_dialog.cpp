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

consumer_t read_text(UserCtx ctx, std::string& text) {
  for (;;) {
    for (auto& e : ctx.events) {
      if (!e.consumed && e.type == EventType::message) {
        text = e.meta["text"].as<std::string>();
        e.consumed = true;
        co_return;
      }
    }
    co_yield {};
  }
}

consumer_t start_dialog(UserCtx ctx) {
  auto user = ctx.db.fetchUser(RequestUser{.user_id = ctx.user_id});
  auto& api = ctx.api;

  std::string name, date;

  consume_first_event(ctx);

  // тут надо бы добавить, что отправка еслси соотвествующий ивент не consumed, нужно крч сделать функцию на
  // сообщение если нет ивента + отправку сообщеиня
  (void)co_await api.sendMessage(
      tgbm::api::send_message_request{.chat_id = user.chat_id, .text = "type your name"});

  co_yield dd::elements_of(read_text(ctx, name));

  (void)co_await api.sendMessage(
      tgbm::api::send_message_request{.chat_id = user.chat_id, .text = "type your date"});

  co_yield dd::elements_of(read_text(ctx, date));

  (void)co_await api.sendMessage(tgbm::api::send_message_request{
      .chat_id = user.chat_id, .text = fmt::format("End dialog. name: {}, date: {}", name, date)});
}

}  // namespace bot

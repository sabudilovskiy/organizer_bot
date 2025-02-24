#pragma once

#include "event_broker.hpp"
#include "types.hpp"
#include "user_context.hpp"
#include <tgbm/logger.hpp>

namespace bot {
template <typename T>
struct Menu {
  using Button = tgbm::api::InlineKeyboardButton;
  using Markup = tgbm::api::InlineKeyboardMarkup;

  struct item {
    std::string text;
    T t;
  };

  Menu(std::string title) noexcept : title_(std::move(title)) {
  }

  Menu& add(std::string text, T t);

  consumer_t show(UserCtx ctx, User& user, T& out) &;

 private:
  std::string title_;
  std::vector<item> items_;
};

template <typename T>
Menu<T>& Menu<T>::add(std::string text, T t) {
  items_.emplace_back(item{
      std::move(text),
      std::move(t),
  });
  return *this;
}

template <typename T>
[[nodiscard]] consumer_t Menu<T>::show(UserCtx ctx, User& user, T& out) & {
  auto& title = title_;
  auto& items = items_;

  auto markup = Markup{};

  for (auto& [item, _] : items_) {
    auto idx = markup.inline_keyboard.size();
    TGBM_LOG("item: {}, idx: {}", item, idx);
    markup.inline_keyboard.emplace_back().emplace_back(Button{
        .text = item,
        .data = Button::callback_data{std::to_string(idx)},
    });
  }

  if (!user.message_id.has_value()) {
    tgbm::api::Message msg = co_await ctx.api.sendMessage(tgbm::api::send_message_request{
        .chat_id = user.chat_id,
        .text = title_,
        .reply_markup = markup,
    });
    user.message_id = msg.message_id;
    ctx.db.updateUser(user);
  } else {
    (void)co_await ctx.api
        .editMessageText(tgbm::api::edit_message_text_request{
            .text = title_,
            .chat_id = user.chat_id,
            .message_id = *user.message_id,
            .reply_markup = std::move(markup),
        })
        .wait();
  }

  for (;;) {
    for (auto& e : ctx.events) {
      e.consumed = true;
      if (e.type == EventType::callback_query) {
        (void)co_await ctx.api
            .answerCallbackQuery(tgbm::api::answer_callback_query_request{
                .callback_query_id = e.meta["id"].as<std::string>(),
            })
            .wait();
        std::int64_t idx = std::stoi(e.meta["data"].as<std::string>());
        if (0 < idx || idx >= items_.size()) {
          TGBM_LOG("idx: {}", idx);
          throw std::runtime_error("Bad idx");
        }
        out = items_[idx].t;
        TGBM_LOG("Menu show co_return");
        co_return;
      }
      TGBM_LOG("Menu show event, id: {}, type: {}", e.event_id, magic_enum::enum_name(e.type));
    }
    TGBM_LOG("yield Menu show, user: {}", user.user_id);
    co_yield {};
  }
}
}  // namespace bot

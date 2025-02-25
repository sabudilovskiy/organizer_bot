#pragma once

#include <scn/scan.h>
#include <tgbm/logger.hpp>

#include "errors.hpp"
#include "event_broker.hpp"
#include "formatters/all.hpp"
#include "types.hpp"
#include "user_context.hpp"
#include "utils.hpp"

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
  auto ts = now();

  for (auto& [item, _] : items_) {
    auto idx = markup.inline_keyboard.size();
    TGBM_LOG("item: {}, idx: {}", item, idx);
    markup.inline_keyboard.emplace_back().emplace_back(Button{
        .text = item,
        .data = Button::callback_data{fmt::format("{};{}", idx, ts)},
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
        auto data = e.meta["data"].as<std::string>();
        auto res = scn::scan<std::size_t, std::string>(data, "{};{}");
        if (!res) {
          TGBM_LOG_CRIT("Menu got bad callback query. Callback data: [{}]", data);
          throw std::runtime_error("Callback query bug");
        }
        auto idx = std::get<0>(res->values());
        if (idx < 0 || idx >= items_.size()) {
          TGBM_LOG_CRIT("Menu got bad callback query, idx: [{}], size: {}, items: [{}]", idx, items_.size(),
                        fmt::join(items_, ","));
          throw std::runtime_error("Callback query bug");
        }
        TGBM_LOG_EVENT("Menu got idx from query : {}", idx);
        out = items_[idx].t;
        TGBM_LOG_EVENT("Menu out: {}", out);
        co_return;
      }
    }
    co_yield {};
  }
}
}  // namespace bot

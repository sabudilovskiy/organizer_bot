#pragma once

#include <scn/scan.h>
#include <tgbm/logger.hpp>

#include "errors.hpp"
#include "io_event_broker.hpp"
#include "io_event_utils.hpp"
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

  Menu(std::string title, std::size_t id) noexcept : title_(std::move(title)), id_(id) {
  }

  Menu& add(std::string text, T t);

  consumer_t show(ContextWithUser ctx, T& out) &;

 private:
  std::string title_;
  std::vector<item> items_;
  std::size_t id_;
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
[[nodiscard]] consumer_t Menu<T>::show(ContextWithUser ctx, T& out) & {
  TGBM_LOG_DEBUG("Create Menu<{}> with id {}", name_type_v<T>, id_);
  auto& user = ctx.user;
  auto& title = title_;
  auto& items = items_;

  std::unordered_map<std::int64_t, T*> cb_map;

  auto markup = Markup{};
  auto ts = now();

  for (auto& [text, value] : items_) {
    auto idx = markup.inline_keyboard.size();
    auto cb_data = fmt::format("{};{};{}", idx, id_, now());
    cb_map[idx] = &value;
    markup.inline_keyboard.emplace_back().emplace_back(Button{
        .text = text,
        .data = Button::callback_data{std::move(cb_data)},
    });
  }

  if (!user.message_id.has_value() || user.need_new_message()) {
    tgbm::api::Message msg = co_await ctx.api.sendMessage({
        .chat_id = user.chat_id,
        .text = title_,
        .reply_markup = markup,
    });
    user.message_id = msg.message_id;
    user.additional_messages = 0;
    ctx.db.updateUser(user);
  } else {
    co_await ctx.api
        .editMessageText({
            .text = title_,
            .chat_id = user.chat_id,
            .message_id = *user.message_id,
            .reply_markup = std::move(markup),
        })
        .wait();
  }

  for (;;) {
    for (auto& e : ctx.events | events::only_cb_queries) {
      e.consumed = true;
      auto& data = e.cb_query_meta().data;
      auto res_scan = scn::scan<std::int64_t, std::size_t, std::string>(data, "{};{};{}");
      if (!res_scan) {
        TGBM_LOG_ERROR("Got cb query in unknown format. Data: [{}]", data);
        continue;
      }
      auto [got_idx, got_id, got_ts] = res_scan->values();
      if (id_ != got_id) {
        TGBM_LOG_WARN("Got cb query with stranger id. Expected_id : {}, Data: [{}]", id_,
                      data);
        continue;
      }
      auto found = cb_map.find(got_idx);
      if (found == cb_map.end()) {
        TGBM_LOG_WARN("Got cb query with stranger idx. Data: [{}]", id_, data);
        continue;
      }
      out = *found->second;
      co_return;
    }
    co_yield {};
  }
}
}  // namespace bot

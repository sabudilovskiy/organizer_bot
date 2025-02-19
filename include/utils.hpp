#pragma once
#include "boost/json.hpp"
#include "tgbm/api/methods/answerCallbackQuery.hpp"
#include "tgbm/api/methods/sendMessage.hpp"
#include "tgbm/api/telegram.hpp"
#include "tgbm/api/types/Update.hpp"

namespace bot {
inline std::string get_string(const boost::json::value& value) {
  return std::string(value.as_string());
}

inline dd::task<void> answer_callback(const tgbm::api::Update& update, tgbm::api::telegram& tg) {
  if (auto query = update.get_callback_query()) {
    (void)co_await tg.answerCallbackQuery(
        tgbm::api::answer_callback_query_request{.callback_query_id = query->id});
  }
}
}  // namespace bot

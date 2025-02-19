

#include <fmt/base.h>
#include <fstream>
#include <memory>
#include <optional>
#include <tgbm/bot.hpp>
#include <tgbm/utils/formatters.hpp>
#include <tgbm/utils/scope_exit.hpp>

#include "consts.hpp"
#include "database.hpp"
#include "event_broker.hpp"
#include "tgbm/logger.hpp"

dd::task<void> start_main_task(tgbm::bot& bot) {
  using namespace bot;
  on_scope_exit {
    // stop bot on failure
    bot.stop();
  };
  Database db("bot.db");
  EventBroker event_broker(bot.api, db);
  event_broker.load();

  fmt::println("launching bot, info: {}", co_await bot.api.getMe());

  co_foreach(tgbm::api::Update && u, bot.updates()) {
    fmt::println("got update, type: [{}]", u.discriminator_now());

    if (auto* msg = u.get_message(); msg->from && msg->from->id && msg->chat->type == "private") {
      db.fetchUser(RequestUser{.user_id = msg->from->id, .chat_id = msg->chat->id});
      json_value meta = json_value::object();
      meta["text"] = msg->text;

      co_await event_broker.add(now(), msg->from->id, EventType::message, meta);
    }
  }
}

int main() {
  std::ifstream token_file("token.txt");
  if (!token_file.is_open()) {
    fmt::println(
        "launching telegram bot requires bot token from @BotFather. Create token.txt and place token.");
    return -1;
  }
  std::string token;
  token_file >> token;
  tgbm::bot bot{token /*"api.telegram.org", "some_ssl_certificate"*/};
  start_main_task(bot).start_and_detach();
  bot.run();
  return 0;
}

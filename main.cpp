

#include <csignal>
#include <fmt/base.h>
#include <fstream>
#include <tgbm/bot.hpp>
#include <tgbm/utils/formatters.hpp>
#include <tgbm/utils/scope_exit.hpp>
#include <tgbm/net/errors.hpp>

#include "database.hpp"
#include "event_broker.hpp"

using namespace bot;

std::atomic_bool need_to_stop{};

void stop_handler(int signal) {
  need_to_stop.store(true, std::memory_order::relaxed);
}

tgbm::long_poll_options lp_options{.timeout = std::chrono::seconds(3)};

dd::task<void> saving_database(tgbm::bot& bot, EventBroker& event_broker) {
  tgbm::io_error_code errc;
  while (!errc) {
    co_await bot.sleep(std::chrono::seconds(3), errc);
    if (need_to_stop.load(std::memory_order::relaxed)) {
      fmt::println("Try to stop bot");
      bot.stop();
      event_broker.save();
      co_return;
    }
    event_broker.save();
  }
}

dd::task<void> start_main_task(tgbm::bot& bot, EventBroker& event_broker) {
  on_scope_exit {
    // stop bot on failure
    bot.stop();
    event_broker.save();
  };

  event_broker.load();

  fmt::println("launching bot, info: {}", co_await bot.api.getMe());
  co_foreach(auto u, bot.updates(lp_options)) {
    co_await event_broker.process_update(std::move(u));
    event_broker.save();
  }
}
int main() {
  on_scope_exit {
    fmt::println("Bot stopped");
  };
  std::ifstream token_file("token.txt");
  if (!token_file.is_open()) {
    fmt::println(
        "launching telegram bot requires bot token from @BotFather. Create token.txt and place token.");
    return -1;
  }
  std::signal(SIGINT, stop_handler);
  std::signal(SIGTERM, stop_handler);
  std::string token;
  token_file >> token;
  tgbm::bot bot{token /*"api.telegram.org", "some_ssl_certificate"*/};
  Database db("bot.db");
  EventBroker event_broker(bot.api, db);
  start_main_task(bot, event_broker).start_and_detach();
  saving_database(bot, event_broker).start_and_detach();
  bot.run();
  return 0;
}

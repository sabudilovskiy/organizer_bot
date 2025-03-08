

#include <csignal>
#include <fmt/base.h>
#include <fstream>
#include <tgbm/bot.hpp>
#include <tgbm/utils/formatters.hpp>
#include <tgbm/utils/scope_exit.hpp>
#include <tgbm/net/errors.hpp>

#include "database.hpp"
#include "io_event_broker.hpp"
#include "tgbm/logger.hpp"

using namespace bot;

std::atomic_bool need_to_stop{};

void stop_handler(int signal) {
  need_to_stop.store(true, std::memory_order::relaxed);
}

tgbm::long_poll_options lp_options{.timeout = std::chrono::seconds(3)};

dd::task<void> saving_database(tgbm::bot& bot, io_event_broker& event_broker) {
  tgbm::io_error_code errc;
  while (!errc) {
    co_await bot.sleep(std::chrono::seconds(3), errc);
    if (need_to_stop.load(std::memory_order::relaxed)) {
      TGBM_LOG_INFO("Try to stop bot...");
      bot.stop();
      event_broker.save();
      co_return;
    }
    event_broker.save();
  }
}

dd::task<void> main_task(tgbm::bot& bot, io_event_broker& event_broker) {
  on_scope_exit {
    bot.stop();
    event_broker.save();
    TGBM_LOG_INFO("End main task");
  };

  auto me = co_await bot.api.getMe();
  TGBM_LOG_INFO("launching bot, info: {}", me);

  co_foreach(auto u, bot.updates(lp_options)) {
    auto update_id = u.update_id;
    co_await event_broker.safe_process_update(std::move(u));
    event_broker.save();
  }
}
int main() try {
  on_scope_exit {
    TGBM_LOG_INFO("Bot stopped.");
  };
  std::ifstream token_file("token.txt");
  if (!token_file.is_open()) {
    TGBM_LOG_CRIT(
        "launching telegram bot requires bot token from @BotFather. Create token.txt and place token.");
    return -1;
  }
  std::signal(SIGINT, stop_handler);
  std::signal(SIGTERM, stop_handler);
  std::string token;
  token_file >> token;
  tgbm::bot bot{token /*"api.telegram.org", "some_ssl_certificate"*/};
  Database db("bot.db");
  io_event_broker event_broker(bot.api, db);
  event_broker.load();
  main_task(bot, event_broker).start_and_detach();
  saving_database(bot, event_broker).start_and_detach();
  bot.run();
  return 0;
} catch (std::exception& exc) {
  TGBM_LOG_CRIT("Start bot failed. Reason : {}", exc.what());
}

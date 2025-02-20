

#include <csignal>
#include <fmt/base.h>
#include <fstream>
#include <tgbm/bot.hpp>
#include <tgbm/utils/formatters.hpp>
#include <tgbm/utils/scope_exit.hpp>

#include "database.hpp"
#include "event_broker.hpp"
#include "long_poll.hpp"
#include "tgbm/logger.hpp"

std::atomic_bool need_to_stop{};

void stop_handler(int signal) {
  need_to_stop.store(true, std::memory_order::relaxed);
}

tgbm::long_poll_options lp_options{.timeout = std::chrono::seconds(3)};

dd::task<void> start_main_task(tgbm::bot& bot) {
  try {
    using namespace bot;
    Database db("bot.db");
    EventBroker event_broker(bot.api, db);
    on_scope_exit {
      // stop bot on failure
      bot.stop();
      event_broker.save();
    };

    event_broker.load();

    fmt::println("launching bot, info: {}", co_await bot.api.getMe());
    co_foreach(auto opt_u, bot::long_poll(bot.api, lp_options)) {
      if (need_to_stop.load(std::memory_order::relaxed)) {
        fmt::println("Stopping bot...");
        bot.stop();
        break;
      }
      if (!opt_u.has_value()) {
        continue;
      }

      co_await event_broker.process_update(std::move(*opt_u));
      event_broker.save();
    }
  } catch (std::exception& exc) {
    TGBM_LOG_ERROR("Got exception: {}", exc.what());
  }
}
int main() {
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
  start_main_task(bot).start_and_detach();
  bot.run();
  fmt::println("Bot stopped");
  return 0;
}

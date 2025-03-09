

#include <csignal>
#include <fmt/base.h>
#include <fstream>
#include <tgbm/bot.hpp>
#include <tgbm/utils/formatters.hpp>
#include <tgbm/utils/scope_exit.hpp>
#include <tgbm/net/errors.hpp>

#include "config.hpp"
#include "database.hpp"
#include "io_event_broker.hpp"
#include "macro.hpp"
#include "organizer_db.hpp"
#include "stop_handler.hpp"
#include "tgbm/logger.hpp"

using namespace bot;

dd::task<void> saving_database(tgbm::bot& bot, io_event_broker& event_broker, const Config& config) {
  tgbm::io_error_code errc;
  bool stopping{};

  while (!errc && !stopping) {
    co_await bot.sleep(std::chrono::seconds(config.interval_save_db), errc);

    try {
      event_broker.save();
    } catch (const std::exception& exc) {
      TGBM_LOG_ERROR("Fail save db: {}", exc.what());
      stopping |= !config.skip_fail_save_db;
    }

    stopping |= is_need_stop();
  }

  TGBM_LOG_INFO("Try to stop bot...");
  try {
    event_broker.save();
  } catch (const std::exception& exc) {
    TGBM_LOG_CRIT("Fail save db before stopping: {}", exc.what());
  }
  bot.stop();
}

dd::task<void> main_task(tgbm::bot& bot, io_event_broker& event_broker, const Config& config) {
  on_scope_exit {
    need_stop();
  };
  auto options = tgbm::long_poll_options{
      .drop_pending_updates = config.skip_fail_updates,
      .timeout = std::chrono::seconds(config.timeout),
  };

  auto me = co_await bot.api.getMe();
  TGBM_LOG_INFO("Bot launched, info: {}", me);

  co_foreach(auto u, bot.updates()) {
    co_await event_broker.process_update(std::move(u));
  }
}

int main() try {
  on_scope_exit {
    TGBM_LOG_INFO("Bot stopped.");
  };
  Config config = read_config_from_fs("config.json");

  tgbm::bot bot{config.token};
  OrganizerDB db("bot.db");
  io_event_broker event_broker(bot.api, db);
  event_broker.load();
  main_task(bot, event_broker, config).start_and_detach();
  saving_database(bot, event_broker, config).start_and_detach();
  bot.run();
  return 0;
} catch (std::exception& exc) {
  TGBM_LOG_CRIT("Start bot failed. Reason : {}", exc.what());
}

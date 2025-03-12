#include "app.hpp"

#include <tgbm/bot.hpp>
#include <tgbm/api/types/all.hpp>
#include <tgbm/utils/formatters.hpp>

#include "formatters/all.hpp"
#include "stop_handler.hpp"

namespace bot {

dd::task<void> App::saving_io_events() {
  tgbm::io_error_code errc;
  bool stopping{};

  while (!errc && !stopping) {
    co_await bot.sleep(std::chrono::seconds(config.interval_saving_io_events), errc);

    try {
      io_ev_broker.save();
    } catch (const std::exception& exc) {
      TGBM_LOG_ERROR("Fail save io_events: {}", exc.what());
      if (!config.skip_fail_save_db) {
        bot.stop();
      }
    }
  }
  try {
    io_ev_broker.save();
  } catch (const std::exception& exc) {
    TGBM_LOG_CRIT("Fail save io_events before stopping: {}", exc.what());
  }
}

dd::task<void> App::checking_stop() {
  tgbm::io_error_code errc;

  set_stop_handler();

  while (!errc && !is_need_stop()) {
    co_await bot.sleep(std::chrono::seconds(config.interval_check_stop), errc);
  }

  TGBM_LOG_INFO("Try to stop bot...");
  bot.stop();
}

dd::task<void> App::listening_updates() {
  on_scope_exit {
    need_stop();
  };

  auto options = tgbm::long_poll_options{
      .timeout = std::chrono::seconds(config.timeout),
  };

  auto me = co_await bot.api.getMe();
  TGBM_LOG_INFO("Bot launched, info: {}", me);

  co_foreach(auto u, bot.updates()) {
    auto u_id = u.update_id;
    try {
      co_await io_ev_broker.process_update(std::move(u));
    } catch (std::exception& exc) {
      TGBM_LOG_ERROR("Fail processing update {}. Error : {}", u_id, exc_what_coro(exc));
    }
  }
}

dd::task<void> App::executing_time_events() {
  tgbm::io_error_code errc;

  while (!errc) {
    co_await bot.sleep(std::chrono::seconds(config.interval_saving_io_events), errc);

    try {
      io_ev_broker.save();
    } catch (const std::exception& exc) {
      TGBM_LOG_ERROR("Fail save executed time events: {}", exc.what());
      if (!config.skip_fail_save_db) {
        bot.stop();
      }
    }
  }
  try {
    io_ev_broker.save();
  } catch (const std::exception& exc) {
    TGBM_LOG_CRIT("Fail save executed time events before stopping: {}", exc.what());
  }
}

App::App(Config config)
    : bot(config.token),
      db(config.db_path),
      io_ev_broker(bot.api, db),
      time_ev_dispatcher(bot.api, db),
      config(std::move(config)) {
  io_ev_broker.load();
  time_ev_dispatcher.load();
}

void App::run() {
  saving_io_events().start_and_detach();
  listening_updates().start_and_detach();
  executing_time_events().start_and_detach();
  checking_stop().start_and_detach();
  io_ev_broker.process_old_events().start_and_detach();
  bot.run();
}

App::~App() {
  TGBM_LOG_INFO("Bot stopped");
}

}  // namespace bot

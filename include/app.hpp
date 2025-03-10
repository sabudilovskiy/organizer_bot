#pragma once

#include <tgbm/bot.hpp>

#include "config.hpp"
#include "io_event_broker.hpp"
#include "time_event_dispatcher.hpp"

namespace bot {
struct App {
  App(Config config);

  dd::task<void> listening_updates();

  dd::task<void> saving_io_events();

  dd::task<void> executing_time_events();

  dd::task<void> checking_stop();

  void run();

  ~App();

 private:
  tgbm::bot bot;
  OrganizerDB db;
  io_event_broker io_ev_broker;
  time_event_dispatcher time_ev_dispatcher;
  Config config;
};
}  // namespace bot

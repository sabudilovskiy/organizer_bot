#pragma once
#include "event_broker.hpp"
#include "user_context.hpp"

namespace bot {

enum struct MainMenu {
  sheduller,
  tasks,
  calls,
  settings,
};

consumer_t main_menu(Context ctx);
consumer_t main_menu(Context ctx, User user);

enum struct ShedulerMenu {
  add,
  list,
  repeatable,
  reminders,
  to_main_menu,
};

consumer_t sheduler_menu(ContextWithUser ctx);

enum struct CallMenu {
  list,
  add,
  settings,
  export_calendar,
  mute,
  to_main_menu,
};

consumer_t call_menu(ContextWithUser ctx);

}  // namespace bot

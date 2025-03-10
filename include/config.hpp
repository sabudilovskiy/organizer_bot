#pragma once

#include <string>
#include <tgbm/api/optional.hpp>

namespace bot {

struct Config {
  std::string token;
  std::string db_path = "organizer_bot.db";
  int64_t timeout = 3;
  int64_t interval_check_stop = 10;
  int64_t interval_exec_time_events = 10;
  int64_t interval_saving_io_events = 10;
  bool skip_fail_save_db = false;
  bool skip_fail_updates = false;
};

Config read_config_from_fs(std::string_view path);

}  // namespace bot

#pragma once

#include <string>
#include <tgbm/api/optional.hpp>

namespace bot {

struct Config {
  std::string token;
  std::string db_path = "organizer_bot.db";
  int64_t timeout = 3;
  int64_t interval_check_stop = 10;
  int64_t interval_consume_time_events = 10;
  int64_t interval_exec_time_events = 10;
  int64_t interval_saving_io_events = 10;
  bool skip_fail_save_db = false;
  bool skip_fail_updates = false;
  bool skip_fail_time_events = false;

  static Config read_from_fs(std::string_view path);
  static void write_default_to_fs(std::string_view path);
};

}  // namespace bot

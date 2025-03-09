#pragma once

#include <string>
#include <tgbm/api/optional.hpp>

namespace bot {

namespace details {
struct OptConfig {
  std::string token;
  tgbm::api::optional<int64_t> timeout;
  tgbm::api::optional<int64_t> interval_save_db;
  tgbm::api::optional<bool> skip_fail_save_db;
  tgbm::api::optional<bool> skip_fail_updates;
};
}  // namespace details

struct Config {
  std::string token;
  int64_t timeout;
  int64_t interval_save_db;
  bool skip_fail_save_db;
  bool skip_fail_updates;
};

inline Config extract_config(details::OptConfig config) {
#define f(X, def) .X = config.X.value_or(def)
  return Config{
      .token = std::move(config.token),
      f(timeout, 3),
      f(interval_save_db, 10),
      f(skip_fail_save_db, false),
      f(skip_fail_updates, true),
  };
#undef f
}

Config read_config_from_fs(const std::string& path);

}  // namespace bot

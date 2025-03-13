#include "config.hpp"

#include <fstream>
#include <tgbm/utils/pfr_extension.hpp>
#include <unordered_set>

#include "json/value.hpp"
#include "tgbm_replace/logger.hpp"
#include "utils.hpp"

namespace bot {

struct OptConfig {
  std::string token;
  tgbm::api::optional<std::string> db_path;
  tgbm::api::optional<int64_t> timeout;
  tgbm::api::optional<int64_t> interval_check_stop;
  tgbm::api::optional<int64_t> interval_exec_time_events;
  tgbm::api::optional<int64_t> interval_saving_io_events;
  tgbm::api::optional<bool> skip_fail_save_db;
  tgbm::api::optional<bool> skip_fail_updates;
};

Config extract_config(OptConfig readed_cfg) {
  Config res{};

  auto m1 = []<auto>(auto& res, const auto& readed) { res = readed; };
  auto m2 = []<auto, typename T>(auto& res, const tgbm::api::optional<T>& readed) {
    if (readed)
      res = readed.value();
  };
  visit_all(tgbm::matcher(m1, m2), res, readed_cfg);
  return res;
}

Config Config::read_from_fs(std::string_view path) {
  std::ifstream file(std::string{path});
  if (!file.is_open()) {
    throw_formatted("Fail read config from: {}", path);
  }
  json_value j(boost::json::parse(file));
  auto fields = []() {
    auto names = boost::pfr::names_as_array<Config>();
    return std::unordered_set<std::string_view>(names.begin(), names.end());
  }();
  auto res = extract_config(j.as<OptConfig>());
  for (auto&& [k, v] : j.value.as_object()) {
    if (!fields.contains(k)) {
      TGBM_LOG_WARN("Ignore '{}' from config, unknown option", k);
    }
  }
  if (res.token.empty()) {
    throw_formatted("In config '{}' missing token", path);
  }
  return res;
}

void Config::write_default_to_fs(std::string_view path) {
  std::ofstream file(std::string{path});
  if (!file.is_open()) {
    throw_formatted("Fail write config to: '{}'", path);
  }
  file << json_value(Config{}).pretty_serialize();
}

}  // namespace bot

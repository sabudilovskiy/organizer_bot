#include "config.hpp"

#include <fstream>

#include "json/value.hpp"
#include "utils.hpp"

namespace bot {

Config read_config_from_fs(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw_formatted("Fail read config from: {}", path);
  }
  json_value j(boost::json::parse(file));
  return extract_config(j.as<details::OptConfig>());
}

}  // namespace bot

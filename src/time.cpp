#include "time.hpp"

namespace bot {

ts_t now() {
  return std::chrono::zoned_time{"UTC", std::chrono::system_clock::now()}.get_local_time();
}

std::string to_string(const ts_t& ts) {
  return std::format("{:%Y-%m-%d %H:%M:%S}", ts);
}

ts_t parse_ts(const std::string& str) {
  std::tm tm = {};
  std::istringstream iss(str);
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  if (iss.fail()) {
    throw std::runtime_error("fail parse");
  }
  std::time_t time = std::mktime(&tm);
  return ts_t(std::chrono::seconds(time));
}

}  // namespace bot

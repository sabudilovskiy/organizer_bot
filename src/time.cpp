#include "time.hpp"

#include <tgbm/utils/macro.hpp>

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

std::string_view to_string(weekday w) noexcept {
  switch (w.iso_encoding()) {
    case 0:
      return "воскресенье";
    case 1:
      return "понедельник";
    case 2:
      return "вторник";
    case 3:
      return "среда";
    case 4:
      return "четверг";
    case 5:
      return "пятница";
    case 6:
      return "суббота";
  }
  tgbm::unreachable();
}
}  // namespace bot

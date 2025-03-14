#include "time.hpp"

#include <tgbm/utils/macro.hpp>
#include <tgbm/utils/string_switch.hpp>
#include <spanstream>

namespace bot {

namespace {

constexpr char fmt_ts_t[] = "%Y-%m-%d %H:%M:%S";
constexpr char fmt_short_ts_t[] = "%d.%m.%Y";

bool ts_from_str(std::string_view str, const char* fmt, ts_t& out) {
  std::tm tm = {};
  std::ispanstream iss(str);
  iss >> std::get_time(&tm, fmt);
  if (iss.fail()) {
    return false;
  }
  std::time_t time = std::mktime(&tm);
  out = ts_t(std::chrono::seconds(time));
  return true;
}

}  // namespace
ts_t now() {
  return std::chrono::zoned_time{"UTC", std::chrono::system_clock::now()}
      .get_local_time();
}

std::string to_string(const ts_t& ts) {
  return std::format("{:%Y-%m-%d %H:%M:%S}", ts);
}

bool parse_ts(const std::string& str, ts_t& out) noexcept {
  return ts_from_str(str, fmt_ts_t, out);
}

bool parse_short_ts(const std::string& str, ts_t& out) noexcept {
  return ts_from_str(str, fmt_short_ts_t, out);
}

optional<ts_t> parse_ts(const std::string& str, use_optional) noexcept {
  optional<ts_t> out;
  if (!ts_from_str(str, fmt_ts_t, out.emplace())) {
    out.reset();
  }
  return out;
}

optional<ts_t> parse_short_ts(const std::string& str, use_optional) noexcept {
  optional<ts_t> out;
  if (!ts_from_str(str, fmt_short_ts_t, out.emplace())) {
    out.reset();
  }
  return out;
}

ts_t parse_ts(const std::string& str) {
  auto res = parse_ts(str, use_optional{});
  if (!res) {
    throw std::runtime_error("fail parse");
  }
  return *res;
}

ts_t parse_short_ts(const std::string& str) {
  auto res = parse_short_ts(str, use_optional{});
  if (!res) {
    throw std::runtime_error("fail parse");
  }
  return *res;
}

optional<weekday> parse_weekday(std::string_view str, use_optional) noexcept {
  return tgbm::string_switch<optional<weekday>>(str)
      .case_("понедельник", std::chrono::Monday)
      .case_("вторник", std::chrono::Tuesday)
      .case_("среда", std::chrono::Wednesday)
      .case_("четверг", std::chrono::Thursday)
      .case_("пятница", std::chrono::Friday)
      .case_("суббота", std::chrono::Saturday)
      .case_("воскресенье", std::chrono::Sunday)
      .or_default(std::nullopt);
}
weekday parse_weekday(std::string_view str) {
  auto res = parse_weekday(str, use_optional{});
  if (!res)
    throw std::runtime_error("failed parse");
  return *res;
}

std::string_view to_string(weekday w) noexcept {
  switch (w.iso_encoding()) {
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
    case 7:
      return "воскресенье";
  }
  tgbm::unreachable();
}

std::string_view human_frequence(schedule_frequence freq) noexcept {
  switch (freq) {
    case schedule_frequence::one_time:
      return "только один раз";
    case schedule_frequence::weekly:
      return "раз в неделю";
    case schedule_frequence::biweekly:
      return "раз в две недели";
  }
  tgbm::unreachable();
}

}  // namespace bot

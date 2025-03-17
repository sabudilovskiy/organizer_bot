#include "time/ts.hpp"

#include <spanstream>

namespace bot {

ts_utc_t::ts_utc_t(native_t time) noexcept : time_(time) {
}

ts_utc_t ts_utc_t::now() noexcept {
  return std::chrono::system_clock::now();
}

ts_utc_t ts_utc_t::parse(std::string_view str, time_zone tz) {
  ts_utc_t out;
  if (!parse(str, tz, out))
    throw std::runtime_error("Can't parse ts");
  return out;
}

bool ts_utc_t::parse(std::string_view str, time_zone tz, ts_utc_t& out) noexcept {
  std::tm tm = {};
  std::ispanstream iss(str);
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  if (iss.fail()) {
    return false;
  }
  std::chrono::local_time<std::chrono::nanoseconds> lt(
      std::chrono::nanoseconds(std::mktime(&tm)));

  out = std::chrono::zoned_time(tz_locate_zone(tz), lt).get_sys_time();
  return true;
}

ts_utc_t::native_t ts_utc_t::native() const noexcept {
  return time_;
}

std::string ts_utc_t::serialize() const {
  return std::format("{:%Y-%m-%d %H:%M:%S}", time_);
}

ts_utc_t ts_utc_t::operator+(std::chrono::nanoseconds ns) const noexcept {
  return time_ + ns;
}

ts_utc_t ts_utc_t::operator-(std::chrono::nanoseconds ns) const noexcept {
  return time_ - ns;
}

ts_utc_t ts_utc_t::never() noexcept {
  return native_t::max();
}

ts_utc_t ts_utc_t::operator+=(std::chrono::nanoseconds ns) noexcept {
  *this = *this + ns;
  return *this;
}

ts_utc_t ts_utc_t::operator-=(std::chrono::nanoseconds ns) noexcept {
  *this = *this + ns;
  return *this;
}

ts_utc_t ts_utc_t::parse(std::string_view str) {
  ts_utc_t out;
  if (!parse(str, out)) {
    throw std::runtime_error("Can't parse ts_utc_t");
  }
  return out;
}

bool ts_utc_t::parse(std::string_view str, ts_utc_t& out) noexcept {
  std::tm tm = {};
  std::ispanstream iss(str);
  iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  if (iss.fail()) {
    return false;
  }
  std::chrono::sys_time<std::chrono::nanoseconds> st(
      std::chrono::nanoseconds(std::mktime(&tm)));

  out = ts_utc_t(st);
  return true;
}

std::chrono::nanoseconds ts_utc_t::operator-(const ts_utc_t& rhs) const noexcept {
  return time_ - rhs.time_;
}
}  // namespace bot

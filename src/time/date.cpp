#include "time/date.hpp"

#include <spanstream>

namespace bot {

bool date::is_valid() const noexcept {
  return ymd_.ok();
}

date::date(native_t ymd) noexcept : ymd_(ymd) {
}

date::date(ts_utc_t ts) noexcept
    : ymd_(native_t(std::chrono::floor<std::chrono::days>(ts.native()))) {
}

date::date(ts_zoned_t ts) noexcept
    : ymd_(
          native_t(std::chrono::floor<std::chrono::days>(ts.native().get_local_time()))) {
}

ts_utc_t date::to_ts() const noexcept {
  return ts_utc_t(std::chrono::sys_days(ymd_));
}

ts_zoned_t date::to_ts_zoned(time_zone tz) const noexcept {
  return ts_zoned_t(std::chrono::local_days(ymd_), tz);
}

date date::parse(std::string_view str) {
  date out;
  if (!parse(str, out)) {
    throw std::runtime_error("fail parse date");
  }
  return out;
}

bool date::parse(std::string_view str, date& out) noexcept {
  std::tm tm = {};
  std::ispanstream iss(str);
  iss >> std::get_time(&tm, "%Y.%m.%d");
  if (iss.fail()) {
    return false;
  }
  std::chrono::sys_time<std::chrono::nanoseconds> time(
      std::chrono::nanoseconds(std::mktime(&tm)));

  out = date(ts_utc_t(time));
  return true;
}

date::native_t date::native() const noexcept {
  return ymd_;
}

std::chrono::days date::operator-(const date& rhs) const noexcept {
  return std::chrono::sys_days(ymd_) - std::chrono::sys_days(rhs.ymd_);
}

date date::operator+(std::chrono::days d) const noexcept {
  return date(std::chrono::sys_days(ymd_) + d);
}

date date::operator-(std::chrono::days d) const noexcept {
  return date(std::chrono::sys_days(ymd_) - d);
}

date& date::operator+=(std::chrono::days d) noexcept {
  *this = *this + d;
  return *this;
}

date& date::operator-=(std::chrono::days d) noexcept {
  *this = *this - d;
  return *this;
}

std::string date::serialize() const {
  return std::format("{:%Y.%m.%d}", ymd_);
}

}  // namespace bot

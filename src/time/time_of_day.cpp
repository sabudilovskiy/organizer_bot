#include "time/time_of_day.hpp"

#include <fmt/format.h>
#include <scn/scan.h>

namespace bot {

time_of_day time_of_day::from_ts(ts_utc_t ts) noexcept {
  using namespace std::chrono;
  auto since_midnight = duration_cast<seconds>(ts.native().time_since_epoch()) % 86400s;

  auto now_time = hh_mm_ss<seconds>(since_midnight);

  return time_of_day{
      now_time.hours().count(),
      now_time.minutes().count(),
  };
}

time_of_day time_of_day::from_ts(ts_zoned_t ts) noexcept {
  using namespace std::chrono;

  auto midnigth = date(ts.native()).to_ts_zoned(ts.zone());

  auto since_midnight = ts - midnigth;

  auto now_time = hh_mm_ss<nanoseconds>(since_midnight);

  return time_of_day{
      now_time.hours().count(),
      now_time.minutes().count(),
  };
}

time_of_day time_of_day::operator+(std::chrono::minutes duration) const noexcept {
  time_of_day result = *this;
  result.minutes_ += duration.count();

  if (result.minutes_ >= 60) {
    result.hour_ += result.minutes_ / 60;
    result.minutes_ %= 60;
  }

  if (result.hour_ >= 24) {
    result.hour_ %= 24;
  }

  return result;
}

std::chrono::minutes time_of_day::operator-(const time_of_day& rhs) const noexcept {
  const auto& lhs = *this;
  int lhs_total_minutes = lhs.hour_ * 60 + lhs.minutes_;
  int rhs_total_minutes = rhs.hour_ * 60 + rhs.minutes_;
  return std::chrono::minutes{lhs_total_minutes - rhs_total_minutes};
}

ts_utc_t time_of_day::to_ts(date date) const noexcept {
  return date.to_ts() + std::chrono::hours(hour_) + std::chrono::minutes(minutes_);
}

ts_zoned_t time_of_day::to_ts_zoned(date date, time_zone tz) const noexcept {
  return date.to_ts_zoned(tz) + std::chrono::hours(hour_) +
         std::chrono::minutes(minutes_);
}

bool time_of_day::is_valid() const noexcept {
  return 0 <= hour_ && hour_ <= 23 && 0 <= minutes_ && minutes_ <= 59;
}

bool time_of_day::is_valid(std::int64_t hour, std::int64_t minute) noexcept {
  return 0 <= hour && hour <= 23 && 0 <= minute && minute <= 59;
}

time_of_day time_of_day::parse(std::string_view str) {
  time_of_day out;
  if (!parse(str, out)) {
    throw std::runtime_error("Can't parse time_of_day");
  }
  return out;
}

bool time_of_day::parse(std::string_view str, time_of_day& out) noexcept {
  auto result = scn::scan<std::int64_t, std::int64_t>(str, "{}:{}");
  if (!result) {
    return false;
  }
  std::tie(out.hour_, out.minutes_) = result->values();
  return out.is_valid();
}

std::string time_of_day::serialize() const {
  return fmt::format("{:02}:{:02}", hour_, minutes_);
}

std::int64_t time_of_day::hour() const noexcept {
  return hour_;
}

std::int64_t time_of_day::minutes() const noexcept {
  return minutes_;
}

time_of_day::time_of_day(std::int64_t hours, std::int64_t minutes) noexcept
    : hour_(hours), minutes_(minutes) {
}
}  // namespace bot

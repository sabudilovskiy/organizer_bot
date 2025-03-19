#pragma once

#include <chrono>

#include "time/ts.hpp"
#include "time/ts_zoned.hpp"

namespace bot {

struct date {
  using native_t = std::chrono::year_month_day;

  date(native_t ymd = {}) noexcept;
  date(ts_utc_t ts) noexcept;
  date(ts_zoned_t ts) noexcept;

  static date parse(std::string_view str);
  static bool parse(std::string_view str, date& out) noexcept;
  std::string serialize() const;

  ts_utc_t to_ts() const noexcept;
  ts_zoned_t to_ts_zoned(time_zone tz) const noexcept;

  bool is_valid() const noexcept;

  native_t native() const noexcept;

  std::chrono::days operator-(const date& rhs) const noexcept;

  date operator+(std::chrono::days d) const noexcept;
  date operator-(std::chrono::days d) const noexcept;
  date& operator+=(std::chrono::days d) noexcept;
  date& operator-=(std::chrono::days d) noexcept;

  auto operator<=>(const date& rhs) const = default;

 private:
  native_t ymd_;
};

}  // namespace bot

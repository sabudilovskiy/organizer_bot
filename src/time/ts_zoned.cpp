#include "time/ts_zoned.hpp"

namespace bot {

ts_zoned_t::ts_zoned_t(ts_utc_t ts, time_zone tz) noexcept
    : time_(tz_locate_zone(tz), ts.native()), tz_(tz) {
}

ts_utc_t ts_zoned_t::to_utc() const noexcept {
  return ts_utc_t(time_.get_sys_time());
}

ts_zoned_t::native_t ts_zoned_t::native() const noexcept {
  return native_t(time_);
}

std::string ts_zoned_t::serialize() const {
  return std::format("{:%z}", time_);
}

time_zone ts_zoned_t::zone() const noexcept {
  return tz_;
}

ts_zoned_t::ts_zoned_t(native_t time) noexcept : time_(time) {
}

ts_zoned_t::ts_zoned_t(local_t time, time_zone tz) noexcept
    : time_(tz_locate_zone(tz), time), tz_(tz) {
}

ts_zoned_t ts_zoned_t::operator+(std::chrono::nanoseconds ns) const noexcept {
  return ts_zoned_t(time_.get_local_time() + ns, tz_);
}

ts_zoned_t ts_zoned_t::operator-(std::chrono::nanoseconds ns) const noexcept {
  return ts_zoned_t(time_.get_local_time() - ns, tz_);
}

std::strong_ordering ts_zoned_t::operator<=>(const ts_zoned_t& rhs) const noexcept {
  return to_utc() <=> rhs.to_utc();
}

ts_zoned_t::local_t ts_zoned_t::local() const noexcept {
  return time_.get_local_time();
}

ts_zoned_t ts_zoned_t::never(time_zone tz) noexcept {
  return ts_zoned_t(local_t::max(), tz);
}

ts_zoned_t ts_zoned_t::now(time_zone tz) noexcept {
  return ts_zoned_t(ts_utc_t::now(), tz);
}

std::chrono::nanoseconds ts_zoned_t::operator-(ts_zoned_t ts) const noexcept {
  return local() - ts.local();
}

ts_zoned_t ts_zoned_t::operator+=(std::chrono::nanoseconds ns) noexcept {
  *this = *this + ns;
  return *this;
}

ts_zoned_t ts_zoned_t::operator-=(std::chrono::nanoseconds ns) noexcept {
  *this = *this - ns;
  return *this;
}
}  // namespace bot

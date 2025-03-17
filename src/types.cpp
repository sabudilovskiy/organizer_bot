#include "types.hpp"

namespace bot {

bool User::need_new_message() {
  return additional_messages >= max_additional_messages;
}

void User::set_need_new_message() {
  additional_messages = max_additional_messages;
}

ts_zoned_t User::now() const noexcept {
  return ts_zoned_t::now(settings.tz);
}

time_of_day Call::begin() const noexcept {
  return schedule.time;
}

time_of_day Call::end() const noexcept {
  return schedule.time + std::chrono::minutes(duration);
}

ts_zoned_t User::convert_to_user_time(ts_utc_t time) const {
  return ts_zoned_t(time, settings.tz);
}

void User::convert_to_user_time(ts_utc_t time, ts_zoned_t& out) const {
  out = convert_to_user_time(time);
}
}  // namespace bot

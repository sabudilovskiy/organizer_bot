#include "types.hpp"

namespace bot {

bool User::need_new_message() {
  return additional_messages >= max_additional_messages;
}

void User::set_need_new_message() {
  additional_messages = max_additional_messages;
}

ts_t User::convert_to_utc(ts_t time) const {
  return time + std::chrono::minutes(settings.gmt_offset_m);
}

ts_t User::convert_to_user_time(ts_t time) const {
  return time - std::chrono::minutes(settings.gmt_offset_m);
}

void User::convert_inplace_to_utc(ts_t& time) const {
  time = convert_to_utc(time);
}

void User::convert_inplace_to_user_time(ts_t& time) const {
  time = convert_to_user_time(time);
}

ts_t User::now() const {
  return ::bot::now() + std::chrono::minutes(settings.gmt_offset_m);
}

time_of_day Call::begin() const noexcept {
  return schedule.time;
}

time_of_day Call::end() const noexcept {
  return schedule.time + std::chrono::minutes(duration);
}

}  // namespace bot

#include "time/weekday.hpp"

#include <magic_enum/magic_enum.hpp>
#include <tgbm/utils/string_switch.hpp>
#include <tgbm/utils/macro.hpp>

namespace bot {

std::string_view to_human_string(weekday_e w) noexcept {
  using enum weekday_e;
  switch (w) {
    case monday:
      return "понедельник";
    case tuesday:
      return "вторник";
    case thursday:
      return "среда";
    case wednesday:
      return "четверг";
    case friday:
      return "пятница";
    case saturday:
      return "суббота";
    case sunday:
      return "воскресенье";
  }
  tgbm::unreachable();
}

weekday_e from_std_weekday(std::chrono::weekday w) noexcept {
  return (weekday_e)w.iso_encoding();
}

weekday::weekday(weekday_e w) noexcept : w_(w) {
}

weekday::weekday(std::chrono::weekday w) noexcept : w_(from_std_weekday(w)) {
}

weekday_e weekday::native() const noexcept {
  return w_;
}

weekday::weekday(date date) noexcept : weekday(std::chrono::weekday(date.native())) {
}

weekday::weekday(ts_utc_t ts) noexcept : weekday(date(ts)) {
}

weekday::weekday(ts_zoned_t ts) noexcept : weekday(date(ts)) {
}

weekday weekday::parse(std::string_view str) {
  weekday out;
  if (!parse(str, out)) {
    throw std::runtime_error("Can't pasre weekday");
  }
  return out;
}

bool weekday::parse(std::string_view str, weekday& out) noexcept {
  auto r = magic_enum::enum_cast<weekday_e>(str);
  if (!r) {
    return false;
  }
  out = *r;
  return true;
}

std::chrono::weekday to_std_weekday(weekday_e w) noexcept {
  int w_i = (int)w;
  if (w_i == 7)
    w_i = 0;
  else
    w_i -= 1;
  return std::chrono::weekday(w_i);
}

std::string_view weekday::serialize() const noexcept {
  return magic_enum::enum_name(w_);
}

}  // namespace bot

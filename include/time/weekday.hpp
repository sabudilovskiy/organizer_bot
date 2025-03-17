#pragma once

#include <chrono>

#include "time/date.hpp"

namespace bot {

enum struct weekday_e {
  monday = 1,
  tuesday,
  wednesday,
  thursday,
  friday,
  saturday,
  sunday,
};

weekday_e parse_weekday(std::string_view str);
bool parse_weekday(std::string_view str, weekday_e& out);

weekday_e from_std_weekday(std::chrono::weekday w) noexcept;
std::chrono::weekday to_std_weekday(weekday_e) noexcept;

std::string_view to_human_string(weekday_e w) noexcept;

struct weekday {
  weekday(weekday_e w = weekday_e::monday) noexcept;
  weekday(std::chrono::weekday w) noexcept;

  weekday(date date) noexcept;

  weekday(ts_utc_t ts) noexcept;
  weekday(ts_zoned_t ts) noexcept;

  static weekday parse(std::string_view str);
  static bool parse(std::string_view str, weekday& out) noexcept;

  std::string_view serialize() const noexcept;

  weekday_e native() const noexcept;

  auto operator<=>(const weekday&) const = default;

 private:
  weekday_e w_;
};

}  // namespace bot

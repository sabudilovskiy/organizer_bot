#pragma once

#include <chrono>
#include <string_view>

namespace bot {

enum struct time_zone {
  kaliningrad = 2,
  moscow,
  samara,
  yekaterinburg,
  omsk,
  krasnoyarsk,
  irkutsk,
  yakutsk,
  vladivostok,
  magadan,
  kamchatka
};

std::string_view tzdb_name(time_zone tz) noexcept;

std::string_view tz_human_name(time_zone tz) noexcept;

const std::chrono::time_zone* tz_locate_zone(time_zone tz);

std::chrono::hours tz_offset_utc(time_zone tz);

}  // namespace bot

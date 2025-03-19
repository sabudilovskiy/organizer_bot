#include "time/zone.hpp"

#include <tgbm/utils/macro.hpp>

namespace bot {

std::string_view tzdb_name(time_zone tz) noexcept {
  switch (tz) {
    case time_zone::kaliningrad:
      return "Europe/Kaliningrad";
    case time_zone::moscow:
      return "Europe/Moscow";
    case time_zone::samara:
      return "Europe/Samara";
    case time_zone::yekaterinburg:
      return "Asia/Yekaterinburg";
    case time_zone::omsk:
      return "Asia/Omsk";
    case time_zone::krasnoyarsk:
      return "Asia/Krasnoyarsk";
    case time_zone::irkutsk:
      return "Asia/Irkutsk";
    case time_zone::yakutsk:
      return "Asia/Yakutsk";
    case time_zone::vladivostok:
      return "Asia/Vladivostok";
    case time_zone::magadan:
      return "Asia/Magadan";
    case time_zone::kamchatka:
      return "Asia/Kamchatka";
  }
  tgbm::unreachable();
}

std::string_view tz_human_name(time_zone tz) noexcept {
  switch (tz) {
    case time_zone::kaliningrad:
      return "Калининградское время (UTC+2)";
    case time_zone::moscow:
      return "Московское время (UTC+3)";
    case time_zone::samara:
      return "Самарское время (UTC+4)";
    case time_zone::yekaterinburg:
      return "Екатеринбургское время (UTC+5)";
    case time_zone::omsk:
      return "Омское время (UTC+6)";
    case time_zone::krasnoyarsk:
      return "Красноярское время (UTC+7)";
    case time_zone::irkutsk:
      return "Иркутское время (UTC+8)";
    case time_zone::yakutsk:
      return "Якутское время (UTC+9)";
    case time_zone::vladivostok:
      return "Владивостокское время (UTC+10)";
    case time_zone::magadan:
      return "Магаданское время (UTC+11)";
    case time_zone::kamchatka:
      return "Камчатское время (UTC+12)";
  }
}

std::chrono::hours tz_offset_utc(time_zone tz) {
  return std::chrono::hours(int(tz));
}

const std::chrono::time_zone* tz_locate_zone(time_zone tz) {
  return std::chrono::locate_zone(tzdb_name(tz));
}

}  // namespace bot

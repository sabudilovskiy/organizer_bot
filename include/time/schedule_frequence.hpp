#pragma once

#include <string_view>

namespace bot {

enum struct schedule_frequence {
  one_time,
  weekly,
  biweekly,
};

std::string_view human_frequence(schedule_frequence freq) noexcept;

}  // namespace bot

#include "time/schedule_frequence.hpp"
#include "tgbm/utils/macro.hpp"

namespace bot {

std::string_view human_frequence(schedule_frequence freq) noexcept {
  switch (freq) {
    case schedule_frequence::one_time:
      return "только один раз";
    case schedule_frequence::weekly:
      return "раз в неделю";
    case schedule_frequence::biweekly:
      return "раз в две недели";
  }
  tgbm::unreachable();
}

}  // namespace bot

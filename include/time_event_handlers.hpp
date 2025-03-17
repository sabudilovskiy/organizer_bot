#pragma once

#include "consumer.hpp"
#include "organizer_db.hpp"
#include "tgbm/api/telegram.hpp"
#include "time_event_dispatcher.hpp"

namespace bot {

[[nodiscard]] consumer_t handle_all_calls(OrganizerDB& db, const tgbm::api::telegram& api,
                                          time_event_dispatcher& dispatcher,
                                          time_event event);
}

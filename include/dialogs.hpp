#pragma once
#include "event_broker.hpp"
#include "user_context.hpp"

namespace bot {

consumer_t start_dialog(Context ctx);

consumer_t create_task(Context ctx);

consumer_t read_text(Context ctx, std::string& text);

}  // namespace bot

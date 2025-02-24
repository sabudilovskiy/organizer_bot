#pragma once
#include "event_broker.hpp"
#include "user_context.hpp"

namespace bot {

consumer_t start_dialog(UserCtx ctx);

consumer_t create_task(UserCtx ctx);

consumer_t read_text(UserCtx ctx, std::string& text);

}  // namespace bot

#pragma once
#include "user_context.hpp"

namespace bot {

consumer_t main_menu(Context ctx, std::int64_t user_id);
consumer_t main_menu(Context ctx, User user);
consumer_t scheduler_menu(ContextWithUser ctx);
consumer_t call_menu(ContextWithUser ctx);
consumer_t call_add_menu(ContextWithUser ctx);
consumer_t call_list_menu(ContextWithUser ctx);
consumer_t call_setting_menu(ContextWithUser ctx);

}  // namespace bot

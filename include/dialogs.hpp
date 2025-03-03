#pragma once
#include "user_context.hpp"

namespace bot {

consumer_t main_menu(Context ctx);
consumer_t main_menu(Context ctx, User user);
consumer_t scheduler_menu(ContextWithUser ctx);
consumer_t call_menu(ContextWithUser ctx);
consumer_t call_add_menu(ContextWithUser ctx);
consumer_t call_list_menu(ContextWithUser ctx);

}  // namespace bot

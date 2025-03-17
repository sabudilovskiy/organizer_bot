#pragma once
#include "user_context.hpp"

namespace bot {

[[nodiscard]] consumer_t main_menu(Context ctx, std::int64_t user_id);
[[nodiscard]] consumer_t main_menu(Context ctx, User user);
[[nodiscard]] consumer_t scheduler_menu(ContextWithUser ctx);
[[nodiscard]] consumer_t call_menu(ContextWithUser ctx);
[[nodiscard]] consumer_t call_add_menu(ContextWithUser ctx);
[[nodiscard]] consumer_t call_list_menu(ContextWithUser ctx);
[[nodiscard]] consumer_t call_setting_menu(ContextWithUser ctx);

}  // namespace bot

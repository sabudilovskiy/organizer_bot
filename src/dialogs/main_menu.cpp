#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {

enum struct MainMenu {
  sheduller,
  tasks,
  calls,
  settings,
};

consumer_t main_menu(Context ctx_, std::int64_t user_id) {
  auto user = ctx_.db.fetchUser(RequestUser{
      .user_id = user_id,
  });
  return main_menu(ctx_, std::move(user));
}

consumer_t main_menu(Context ctx_, User user) {
  ContextWithUser ctx(ctx_, user);
  for (;;) {
    MainMenu choosed;
    auto menu = Menu<MainMenu>{
        "Привет! Я ваш органайзер-бот. Помогу не забыть важные задачи, напомню о "
        "созвонах и помогу с "
        "планированием дня. Чем могу помочь?",
        ID()};
    menu.add("📅 Планировщик", MainMenu::sheduller)
        .add("📞 Созвоны ", MainMenu::calls)
        .add("✅ Срочные задачи", MainMenu::tasks)
        .add("⚙️ Настройки", MainMenu::settings);
    AWAIT_ALL(menu.show(ctx, choosed));
    switch (choosed) {
      case MainMenu::sheduller:
        AWAIT_ALL(scheduler_menu(ctx));
        break;
      case MainMenu::tasks:
        co_await ctx.send_text("Пока что не реализовано 😔");
        break;
      case MainMenu::calls:
        AWAIT_ALL(call_menu(ctx));
        break;
      case MainMenu::settings:
        co_await ctx.send_text("Пока что не реализовано 😔");
        break;
    }
  }
}

}  // namespace bot

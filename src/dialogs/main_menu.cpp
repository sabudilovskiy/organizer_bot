#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {

consumer_t main_menu(Context ctx_) {
  auto user = ctx_.db.fetchUser(RequestUser{.user_id = ctx_.user_id});
  return main_menu(ctx_, std::move(user));
}

consumer_t main_menu(Context ctx_, User user) {
  ContextWithUser ctx(ctx_, user);

  MainMenu choosed;
  auto menu = Menu<MainMenu>{
      "Привет! Я ваш органайзер-бот. Помогу не забыть важные задачи, напомню о созвонах и помогу с "
      "планированием дня. Чем могу помочь?"};
  menu.add("📅 Планировщик", MainMenu::sheduller)
      .add("📞 Созвоны ", MainMenu::calls)
      .add("✅ Срочные задачи", MainMenu::tasks)
      .add("⚙️ Настройки", MainMenu::settings);
  $await_all(menu.show(ctx, choosed));
  switch (choosed) {
    case MainMenu::sheduller:
      $await_all(sheduler_menu(ctx));
      co_return;
    case MainMenu::tasks:
    case MainMenu::calls:
    case MainMenu::settings:
      $await_all(ctx.send_text("Пока что не реализовано 😔"));
      co_return;
  }
}

}  // namespace bot

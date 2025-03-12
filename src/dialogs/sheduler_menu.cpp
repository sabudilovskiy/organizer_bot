#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {

enum struct schedulerMenu {
  add,
  list,
  repeatable,
  reminders,
  to_main_menu,
};

consumer_t scheduler_menu(ContextWithUser ctx) {
  for (;;) {
    schedulerMenu choosed;
    auto menu = Menu<schedulerMenu>{
        "📌 Здесь вы можете управлять своими задачами и напоминаниями", ID()};
    // clang-format off
  menu.add("➕ Добавить задачу",      schedulerMenu::add)
      .add("📜 Список задач ",        schedulerMenu::list)
      .add("🔄 Повторяющиеся задачи", schedulerMenu::repeatable)
      .add("🔔 Напоминания",          schedulerMenu::reminders)
      .add("🏠 Главное меню",         schedulerMenu::to_main_menu);
    // clang-format on
    AWAIT_ALL(menu.show(ctx, choosed));
    switch (choosed) {
      case schedulerMenu::add:
      case schedulerMenu::list:
      case schedulerMenu::repeatable:
      case schedulerMenu::reminders:
        co_await ctx.send_text("Пока что не реализовано 😔");
        break;
      case schedulerMenu::to_main_menu:
        co_return;
    }
  }
}
}  // namespace bot

#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {
consumer_t sheduler_menu(ContextWithUser ctx) {
  ShedulerMenu choosed;
  auto menu = Menu<ShedulerMenu>{"📌 Здесь вы можете управлять своими задачами и напоминаниями"};
  // clang-format off
  menu.add("➕ Добавить задачу",      ShedulerMenu::add)
      .add("📜 Список задач ",        ShedulerMenu::list)
      .add("🔄 Повторяющиеся задачи", ShedulerMenu::repeatable)
      .add("🔔 Напоминания",          ShedulerMenu::reminders)
      .add("🏠 Главное меню",         ShedulerMenu::main_menu);
  // clang-format on
  $await_all(menu.show(ctx, choosed));
  switch (choosed) {
    case ShedulerMenu::add:
    case ShedulerMenu::list:
    case ShedulerMenu::repeatable:
    case ShedulerMenu::reminders:
      $await_all(ctx.send_text("Пока что не реализовано 😔"));
    case ShedulerMenu::main_menu:
      co_return;
  }
}
}  // namespace bot

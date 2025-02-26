#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {

using e = CallMenu;

consumer_t call_menu(ContextWithUser ctx) {
  e choosed;
  auto menu = Menu<e>{
      "Этот раздел поможет вам не забыть о важных созвонах и гибко настроить уведомления под свои "
      "предпочтения. Бот будет следить за расписанием, отправлять напоминания и запрашивать подтверждение, "
      "чтобы вы точно не пропустили важный звонок. \n📝 Доступные возможности:"};
  menu.add("📜 Список созвонов", e::list)
      .add("➕ Новый созвон", e::add)
      .add("⏰ Настройки уведомлений", e::settings)
      .add("📅 Синхронизация с календарём ", e::export_calendar)
      .add("🔕 Отключить уведомления", e::mute)
      .add("🏠 Главное меню ", e::to_main_menu);
  $await_all(menu.show(ctx, choosed));
  switch (choosed) {
    case CallMenu::list:
    case CallMenu::add:
    case CallMenu::settings:
    case CallMenu::export_calendar:
    case CallMenu::mute:
      $await_all(ctx.send_text("Пока что не реализовано 😔"));
      co_return;
    case CallMenu::to_main_menu:
      co_return;
  }
}

}  // namespace bot

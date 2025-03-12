#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {

enum struct CallMenu {
  list,
  add,
  settings,
  export_calendar,
  mute,
  to_main_menu,
};

using e = CallMenu;

consumer_t call_menu(ContextWithUser ctx) {
  for (;;) {
    e choosed;
    auto menu = Menu<e>{
        "Этот раздел поможет вам не забыть о важных созвонах и гибко настроить "
        "уведомления под свои "
        "предпочтения. Бот будет следить за расписанием, отправлять напоминания и "
        "запрашивать подтверждение, "
        "чтобы вы точно не пропустили важный звонок. \n📝 Доступные возможности:",
        ID()};
    menu.add("📜 Список созвонов", e::list)
        .add("➕ Новый созвон", e::add)
        .add("⏰ Настройки уведомлений", e::settings)
        .add("📅 Синхронизация с календарём ", e::export_calendar)
        .add("🔕 Отключить уведомления", e::mute)
        .add("🏠 Главное меню ", e::to_main_menu);

    AWAIT_ALL(menu.show(ctx, choosed));
    switch (choosed) {
      case CallMenu::add:
        AWAIT_ALL(call_add_menu(ctx));
        break;
      case CallMenu::list:
        AWAIT_ALL(call_list_menu(ctx));
        break;
      case CallMenu::settings:
      case CallMenu::export_calendar:
      case CallMenu::mute:
        co_await ctx.send_text("Пока что не реализовано 😔");
        break;
      case CallMenu::to_main_menu:
        co_return;
    }
  }
}

}  // namespace bot

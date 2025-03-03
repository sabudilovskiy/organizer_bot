#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {

namespace {
constexpr std::size_t one_page = 5;

enum struct ListMenu {
  first,
  second,
  third,
  fourth,
  fifth,
  prev_page,
  next_page,
  settings,
  to_main_menu,
};

}  // namespace

consumer_t call_list_menu(ContextWithUser ctx) {
  auto calls = ctx.db.getCalls(ctx.user_id);

  if (calls.empty()) {
    co_await ctx.send_text(
        "📭 У вас пока нет запланированных созвонов. 📌 Созвоны помогают не забывать о встречах и заранее "
        "получать напоминания. Добавьте первый созвон, и я помогу вам его не пропустить!");
    co_return;
  }

  std::size_t cur_page = 0;
  std::size_t pages = (calls.size() / one_page) + (calls.size() % one_page != 0);

  for (;;) {
    Menu<ListMenu> menu(fmt::format("📌 Ваши запланированные созвоны ({}/{}):", cur_page + 1, pages), ID());
    std::size_t first_on_this_page = 5 * cur_page;
    std::size_t last_on_this_page = std::min(4 + 5 * cur_page, calls.size());
    for (std::size_t i = first_on_this_page, idx = 0; i < last_on_this_page; i++, idx++) {
      auto& call = calls[i];
      menu.add(fmt::format("🔹 {}. {} — {}, {}, {}", idx + 1, call.name, call.schedule.weekday,
                           call.schedule.time, human_frequence(call.schedule.frequence)),
               ListMenu(idx));
    }
    menu.add("⬅️ Предыдущая страница", ListMenu::prev_page);
    menu.add("➡️ Следующая страница", ListMenu::next_page);
    menu.add("⚙️ Настройки", ListMenu::settings);
    menu.add("🏠 Главное меню", ListMenu::to_main_menu);

    ListMenu choosed;
    AWAIT_ALL(menu.show(ctx, choosed));
    switch (choosed) {
      case ListMenu::first:
      case ListMenu::second:
      case ListMenu::third:
      case ListMenu::fourth:
      case ListMenu::fifth:
        co_await ctx.send_text("Пока что не реализовано 😔");
      case ListMenu::prev_page:
        cur_page = std::max<std::size_t>(cur_page, 1) - 1;
        break;
      case ListMenu::next_page:
        cur_page = std::min(cur_page + 1, pages);
        break;
      case ListMenu::settings:
        co_await ctx.send_text("Пока что не реализовано 😔");
      case ListMenu::to_main_menu:
        ctx.to_main_menu();
        co_yield {};
        co_return;
    }
  }
}

}  // namespace bot

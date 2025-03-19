#include "dialogs.hpp"
#include "macro.hpp"
#include "menu.hpp"
#include "formatters/all.hpp"

namespace bot {

namespace {

enum struct SettingsMenu {
  daily_notify,
  confirmation,
  time_zone,
  back,
  to_main_menu,
};

enum struct DailyNotifyMenu {
  add_time,
  remove_time,
  back,
};

time_event get_user_all_notify(ContextWithUser ctx) {
  auto time_events = ctx.db.getUserTimeEventsByType(ctx.user.user_id,
                                                    time_event_type::reminder_all_calls);

  if (time_events.size() != 0) {
    return std::move(time_events.front());
  } else {
    return time_event{
        .user_id = ctx.user.user_id,
        .meta = reminder_all_calls_meta_t(),
    };
  }
}

}  // namespace

consumer_t edit_notify_point(ContextWithUser ctx, std::set<time_of_day>& tps,
                             std::size_t idx) {
  auto menu = Menu<>{"⏰ Изменение времени уведомления", ID()};
  menu.add("🕒 Указать новое время", 0);
  menu.add("❌ Удалить это время", 1);
  menu.add("🔙 Назад", 2);

  std::size_t choosed;
  AWAIT_ALL(menu.show(ctx, choosed));

  if (choosed == 0) {
    tps.erase(std::next(tps.begin(), idx));
    time_of_day td;
    AWAIT_ALL(ctx.read_time("Введите новое время уведомления (чч:мм):", td));
    tps.emplace(td);
  } else if (choosed == 1) {
    tps.erase(std::next(tps.begin(), idx));
  }
}

consumer_t add_notify_point(ContextWithUser ctx, std::set<time_of_day>& tps) {
  time_of_day td;
  AWAIT_ALL(ctx.read_time("Введите время нового уведомления (чч:мм):", td));
  co_await ctx.send_text("✅ Новое уведомление добавлено!");
  tps.emplace(td);
}

void save_time_points(ContextWithUser ctx, time_event& te) {
  te.next_occurence =
      te.reminder_all_calls_meta().next_occurence(ctx.user.now()).to_utc();

  ctx.time_event_dispatcher.consume(te.time_event_id);
  te.time_event_id = ctx.time_event_dispatcher.push(te);
  ctx.time_event_dispatcher.save();
}

consumer_t daily_notify_settings(ContextWithUser ctx) {
  time_event te = get_user_all_notify(ctx);
  auto& notify_times = te.reminder_all_calls_meta().time_points;
  for (;;) {
    auto idx_add = notify_times.size();
    auto idx_back = idx_add + 1;
    auto idx_main = idx_back + 1;

    std::string menu_text = "⏰ Ежедневные уведомления\n\n";
    if (notify_times.empty()) {
      menu_text += "❌ У вас пока нет настроенных уведомлений.\n";
    } else {
      menu_text += "📅 Текущие уведомления:\n";
    }

    auto menu = Menu<>{menu_text, ID()};
    for (std::size_t i = 0; auto& notify : notify_times) {
      menu.add(fmt::format("{}", notify), i++);
    }
    menu.add("🕒 Добавить уведомление", idx_add);
    menu.add("🔙 Назад ", idx_back);
    menu.add("🏠 Главное меню ", idx_main);
    std::size_t choosed;
    AWAIT_ALL(menu.show(ctx, choosed));
    if (0 <= choosed && choosed < notify_times.size()) {
      AWAIT_ALL(edit_notify_point(ctx, notify_times, choosed));
      save_time_points(ctx, te);
    } else if (choosed == idx_add) {
      AWAIT_ALL(add_notify_point(ctx, notify_times));
      save_time_points(ctx, te);
    } else if (choosed == idx_back) {
      co_return;
    } else {
      ctx.to_main_menu();
      co_yield {};
    }
  }
}

consumer_t call_setting_menu(ContextWithUser ctx) {
  using E = SettingsMenu;
  E choosed;
  Menu<E> menu(R"(⚙️ Настройки уведомлений
Настройте уведомления, чтобы не пропустить важные созвоны! 😊)",
               ID());
  menu.add("⏰ Ежедневные уведомления", E::daily_notify);
  menu.add("✅ Подтверждение участия",
           E::confirmation);  // TODO тут надо либо крестик, либо галочку
                              // писать в зависимости от текущих настроек
                              // пока оставим так, ибо подтверждение не реализовано и
                              // непонятно когда будет
  menu.add("🌍 Часовой пояс", E::time_zone);
  menu.add("🔙 Назад ", E::back);
  menu.add("🏠 Главное меню ", E::to_main_menu);

  AWAIT_ALL(menu.show(ctx, choosed));

  switch (choosed) {
    case SettingsMenu::daily_notify:
      AWAIT_ALL(daily_notify_settings(ctx));
      break;
    case SettingsMenu::confirmation:
    case SettingsMenu::time_zone:
      co_await ctx.send_text("Пока что не реализовано 😔");
      break;
    case SettingsMenu::back:
      co_return;
    case SettingsMenu::to_main_menu:
      ctx.to_main_menu();
      co_yield {};
      break;
  }
}
}  // namespace bot

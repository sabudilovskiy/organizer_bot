#include "dialogs.hpp"
#include "io_event_utils.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {

using namespace std::chrono;

[[nodiscard]] consumer_t weekday_input(ContextWithUser ctx, weekday& out) {
  ctx.set_need_new_message();
  Menu<weekday> weekday_menu("📆 Выберите день недели, в который будет проходить созвон:",
                             ID());
  // clang-format off
    weekday_menu.add("Понедельник", Monday);
    weekday_menu.add("Вторник"    , Tuesday);
    weekday_menu.add("Среда"      , Wednesday);
    weekday_menu.add("Четверг"    , Thursday);
    weekday_menu.add("Пятница"    , Friday);
    weekday_menu.add("Суббота"    , Saturday);
    weekday_menu.add("Воскресенье", Sunday);
  // clang-format on
  AWAIT_ALL(weekday_menu.show(ctx, out));
  ctx.set_need_new_message();
}

[[nodiscard]] consumer_t begin_input(ContextWithUser ctx, time_of_day& out) {
  AWAIT_ALL(ctx.read_time(
      "⏰ «Введите время начала созвона в формате ЧЧ:ММ (например, 15:30):", out));
}

[[nodiscard]] consumer_t duration_input(ContextWithUser ctx, int64_t& out) {
  AWAIT_ALL(ctx.read_positive_number("⏰ «Введите длительность созвона в минутах:", out));
}

[[nodiscard]] consumer_t active_from(ContextWithUser ctx, date& out) {
  std::string input;
  AWAIT_ALL(ctx.read_text(
      "📅 Введите дату первого созвона в формате ДД.ММ.ГГГГ (например, 06.03.2025):",
      input));
  ;
  while (!date::parse(input, out)) {
    AWAIT_ALL(ctx.read_text(
        "⚠️ Ошибка ввода! Дата должна быть в формате ДД.ММ.ГГГГ (например, 06.03.2025):"
        "🔄 Попробуйте ещё раз и введите время в правильном формате: ",
        input));
  }
}

[[nodiscard]] consumer_t schedule_frequence_input(ContextWithUser ctx,
                                                  schedule_frequence& out) {
  ctx.set_need_new_message();
  Menu<schedule_frequence> menu("⏳ Как часто будет проводиться этот созвон?", ID());
  // clang-format off
  menu.add("🎯 Только один раз", schedule_frequence::one_time);
  menu.add("🔄 Раз в неделю", schedule_frequence::weekly);
  menu.add("🔃 Раз в две недели", schedule_frequence::biweekly);
  // clang-format on
  AWAIT_ALL(menu.show(ctx, out));
}

[[nodiscard]] consumer_t call_add_menu(ContextWithUser ctx) {
  Call call;
  call.user_id = ctx.user.user_id;
  AWAIT_ALL(ctx.read_text(
      "📌 Введите название созвона. Оно поможет вам быстро ориентироваться в расписании.",
      call.name));

  AWAIT_ALL(
      ctx.read_text("📝 Добавьте описание созвона (можно пропустить, отправив -). "
                    "Описание поможет уточнить "
                    "тему или повестку встречи.",
                    call.description));
  if (call.description == "-")
    call.description = "";

  AWAIT_ALL(duration_input(ctx, call.duration));

  std::vector<schedule_unit> schedule_units;
  co_await ctx.send_text(
      "🕒 Теперь настроим расписание для созвона. Можно добавить несколько вариантов, "
      "если "
      "встреча проводится в разные дни или в разное время.");

  for (;;) {
    schedule_unit su;
    AWAIT_ALL(weekday_input(ctx, su.wd));
    AWAIT_ALL(begin_input(ctx, su.time));
    AWAIT_ALL(schedule_frequence_input(ctx, su.frequence));
    AWAIT_ALL(active_from(ctx, su.active_from));

    schedule_units.emplace_back(std::move(su));
    if (schedule_units.size() == 1) {
      co_await ctx.send_text(
          "✅ Первая единица расписания создана! Если созвон проходит несколько раз в "
          "неделю или в разное "
          "время, то можно добавить ещё расписание.");
    } else {
      co_await ctx.send_text("✅ Единица расписания добавлена!");
    }

    bool need_more;

    Menu<bool> need_more_menu("Хотите добавить ещё одно расписание для этого созвона?",
                              ID());
    need_more_menu.add("✅ Добавить ещё", true);
    need_more_menu.add("❌ Завершить", false);
    AWAIT_ALL(need_more_menu.show(ctx, need_more));
    if (!need_more) {
      break;
    }
  }
  for (auto& su : schedule_units) {
    call.schedule = std::move(su);
    ctx.db.addCall(call);
  }
  co_await ctx.send_text("✅ Созвоны успешно добавлены!");
}
}  // namespace bot

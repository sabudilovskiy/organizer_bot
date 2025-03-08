#include "dialogs.hpp"
#include "io_event_utils.hpp"
#include "macro.hpp"
#include "menu.hpp"

namespace bot {

using namespace std::chrono;

consumer_t weekday_input(ContextWithUser ctx, weekday& out) {
  ctx.set_need_new_message();
  Menu<weekday> weekday_menu("📆 Выберите день недели, в который будет проходить созвон:", ID());
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

consumer_t time_of_day_input(ContextWithUser ctx, time_of_day& out) {
  out = time_of_day{-1, -1};
  std::string input;
  AWAIT_ALL(ctx.read_text("⏰ «Введите время начала созвона в формате ЧЧ:ММ (например, 15:30):", input));
  auto result = scn::scan<int, int>(input, "{:d}:{:d}");
  bool failed = !result.has_value();
  if (!failed) {
    auto [h, m] = result->values();
    out = time_of_day{h, m};
  }
  while (!out.is_valid()) {
    AWAIT_ALL(
        ctx.read_text("⚠️ Ошибка ввода! Время должно быть в формате ЧЧ:ММ (например, 15:30)."
                      "🔄 Попробуйте ещё раз и введите время в правильном формате: ",
                      input));
    result = scn::scan<int, int>(input, "{:d}:{:d}");
    failed = !result.has_value();
    if (!failed) {
      auto [h, m] = result->values();
      out = time_of_day{h, m};
    }
  }
}

consumer_t start_date(ContextWithUser ctx, ts_t& out) {
  std::string input;
  optional<ts_t> res;
  AWAIT_ALL(
      ctx.read_text("📅 Введите дату первого созвона в формате ДД.ММ.ГГГГ (например, 06.03.2025):", input));
  res = parse_short_ts(input, use_optional{});
  while (!res) {
    AWAIT_ALL(
        ctx.read_text("⚠️ Ошибка ввода! Дата должна быть в формате ДД.ММ.ГГГГ (например, 06.03.2025):"
                      "🔄 Попробуйте ещё раз и введите время в правильном формате: ",
                      input));
    res = parse_short_ts(input, use_optional{});
  }
  out = *res;
}

consumer_t schedule_frequence_input(ContextWithUser ctx, schedule_frequence& out) {
  ctx.set_need_new_message();
  Menu<schedule_frequence> menu("⏳ Как часто будет проводиться этот созвон?", ID());
  // clang-format off
  menu.add("🎯 Только один раз", schedule_frequence::one_time);
  menu.add("🔄 Раз в неделю", schedule_frequence::weekly);
  menu.add("🔃 Раз в две недели", schedule_frequence::biweekly);
  // clang-format on
  AWAIT_ALL(menu.show(ctx, out));
}

consumer_t call_add_menu(ContextWithUser ctx) {
  Call call;
  call.user_id = ctx.user_id;
  AWAIT_ALL(ctx.read_text("📌 Введите название созвона. Оно поможет вам быстро ориентироваться в расписании.",
                          call.name));

  AWAIT_ALL(
      ctx.read_text("📝 Добавьте описание созвона (можно пропустить, отправив -). Описание поможет уточнить "
                    "тему или повестку встречи.",
                    call.description));
  if (call.description == "-")
    call.description = "";
  std::vector<schedule_unit> schedule_units;
  co_await ctx.send_text(
      "🕒 Теперь настроим расписание для созвона. Можно добавить несколько вариантов, если "
      "встреча проводится в разные дни или в разное время.");

  for (;;) {
    schedule_unit su;
    AWAIT_ALL(weekday_input(ctx, su.weekday));
    AWAIT_ALL(time_of_day_input(ctx, su.time));
    AWAIT_ALL(schedule_frequence_input(ctx, su.frequence));
    AWAIT_ALL(start_date(ctx, su.start_date));

    schedule_units.emplace_back(std::move(su));
    if (schedule_units.size() == 1) {
      co_await ctx.send_text(
          "✅ Первая единица расписания создана! Если созвон проходит несколько раз в неделю или в разное "
          "время, то можно добавить ещё расписание.");
    } else {
      co_await ctx.send_text("✅ Единица расписания добавлена!");
    }

    bool need_more;

    Menu<bool> need_more_menu("Хотите добавить ещё одно расписание для этого созвона?", ID());
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

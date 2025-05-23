#include "time_event_dispatcher.hpp"

namespace bot {
namespace {
std::string_view state_emoji(state_today st) {
  switch (st) {
    case state_today::past:
      return "🔴";
    case state_today::upcoming:
      return "🟡";
    case state_today::active:
      return "🟢";
    case state_today::other_day:
      return "⚫";
  }
}

using today_calls_comp = decltype([](const Call& lhs, const Call& rhs) {
  return lhs.schedule.time < rhs.schedule.time;
});

using today_calls_t = std::map<Call, std::string_view, today_calls_comp>;

today_calls_t get_today_calls(const User& user, OrganizerDB& db) {
  auto calls = db.getCalls(user.user_id);
  auto ts = user.now();

  today_calls_t result;

  for (auto& call : calls) {
    auto st = call.schedule.get_state_today(ts);
    if (st != state_today::other_day) {
      result[std::move(call)] = state_emoji(st);
    }
  }
  return result;
}

[[nodiscard]] action_t handle_zero_calls(const User& user,
                                         const tgbm::api::telegram& api) {
  std::string txt = R"(
🔔 Ежедневное уведомление о созвонах 🔔

На сегодня у вас нет запланированных созвонов. 📭

Но вы можете:
📅 Добавить новый созвон — чтобы не забыть о важной встрече.
⏰ Настроить уведомления — чтобы бот напоминал вам в нужное время.
📜 Просмотреть список созвонов — убедиться, что ничего не упущено.

Чтобы изменить настройки или добавить новый созвон, зайдите в меню 📲

Хорошего дня! 😊
  )";
  (void)co_await api.sendMessage({
      .chat_id = user.chat_id,
      .text = std::move(txt),
  });
}

void reschedule(const User& user, time_event& te, TimeEventDispatcher& dispatcher) {
  te.next_occurence = te.reminder_all_calls_meta()
                          .next_occurence(user.now() + std::chrono::minutes(1))
                          .to_utc();
  dispatcher.consume(te.time_event_id);
  dispatcher.push(te);
  dispatcher.save();
}

}  // namespace

consumer_t handle_all_calls(OrganizerDB& db, const tgbm::api::telegram& api,
                            TimeEventDispatcher& dispatcher, time_event event) {
  assert(event.type() == time_event_type::reminder_all_calls);

  auto user = db.fetchUser(RequestUser{
      .user_id = event.user_id.value(),
  });
  reschedule(user, event, dispatcher);

  auto today_calls = get_today_calls(user, db);

  if (today_calls.empty()) {
    co_await handle_zero_calls(user, api);
    co_return;
  }

  std::string txt = R"(📅 Сегодняшние созвоны
      
        🔔 Как читать цвета?
        
        🔴 — Уже прошедшие созвоны
        🟢 — Текущие созвоны (идут в данный момент)
        🟡 — Будущие созвоны (ещё впереди)
        
        Если у вас сегодня запланированы созвоны, бот будет присылать напоминания вовремя. 
        Не забудьте проверить расписание, чтобы ничего не пропустить! ⏰)";

  std::size_t idx = 0;
  for (auto& [call, emo] : today_calls) {
    auto line = fmt::format("\n\n{} {} - {} — {} ({})", emo, call.begin(), call.end(),
                            call.name, human_frequence(call.schedule.frequence));
    txt.append(std::move(line));
  }

  (void)co_await api.sendMessage({
      .chat_id = user.chat_id,
      .text = std::move(txt),
  });
}
}  // namespace bot

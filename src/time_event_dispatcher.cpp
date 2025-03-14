#include "time_event_dispatcher.hpp"
#include "menu.hpp"
#include "user_context.hpp"

namespace bot {

namespace {

std::string_view state_emoji(state_today st) {
  switch (st) {
    case state_today::past:
      return "🔴";
    case state_today::other_day:
      return "🟡";
    case state_today::active:
      return "🟢";
    default:
      tgbm::unreachable();
  }
}

consumer_t handle_all_calls(OrganizerDB& db, const tgbm::api::telegram& api,
                            time_event event) {
  auto user = db.fetchUser(RequestUser{
      .user_id = event.user_id.value(),
  });

  auto calls = db.getCalls(user.user_id);
  auto ts = user.now();

  calls.erase(
      std::remove_if(calls.begin(), calls.end(),
                     [&](Call& call) {
                       user.convert_inplace_to_user_time(call.schedule.start_date);
                       auto st = call.schedule.get_state_today(ts);
                       return st == state_today::other_day;
                     }),
      calls.end());
  std::ranges::sort(calls, [](const Call& lhs, const Call& rhs) {
    return lhs.schedule.time < rhs.schedule.time;
  });
  std::string txt = R"(📅 Сегодняшние созвоны

  🔔 Как читать цвета?
  
  🔴 — Уже прошедшие созвоны
  🟢 — Текущие созвоны (идут в данный момент)
  🟡 — Будущие созвоны (ещё впереди)
  
  👇 Выбери созвон, чтобы узнать детали или подтвердить участие: )";

  std::size_t idx = 0;
  for (auto& call : calls) {
    auto st = call.schedule.get_state_today(ts);
    auto emo = state_emoji(st);

    auto txt = fmt::format("{} {} - {} — {} ({})", emo, call.begin(), call.end(),
                           call.description, human_frequence(call.schedule.frequence));
    txt.append(txt);
  }

  (void)co_await api.sendMessage({
      .chat_id = user.chat_id,
      .text = std::move(txt),
  });
}

}  // namespace

void time_event_dispatcher::push(time_event event) {
  event.time_event_id = db.addTimeEvent(event);
  queue.push(std::move(event));
}

void time_event_dispatcher::save() {
  db.consumeTimeEvents(consumed_events);
  consumed_events.clear();
  TGBM_LOG_INFO("Saved {} consumed time_events", consumed_events.size());
}

void time_event_dispatcher::load() {
  auto events = db.getTimeEvents(now() + std::chrono::hours(1));
  for (auto& e : events) {
    queue.push(std::move(e));
  }
}

consumer_t time_event_dispatcher::handle(time_event event) {
  auto id = event.time_event_id;
  switch (event.type()) {
    case time_event_type::reminder_all_calls:
      handle_all_calls(db, api, std::move(event));
      break;
    case time_event_type::reminder_call:
      break;
  }
  consumed_events.emplace(id);
  co_return;
}

const time_event* time_event_dispatcher::top() {
  while (!queue.empty() && consumed_events.contains(queue.top().time_event_id)) {
    queue.pop();
  }
  return !queue.empty() ? &queue.top() : nullptr;
}

ts_t time_event_dispatcher::next_occurenece() {
  auto t = top();
  return t ? t->next_occurence : ts_t::max();
}

consumer_t time_event_dispatcher::execute() {
  auto now_v = now();
  while (top() && top()->next_occurence - now_v >= std::chrono::seconds(1)) {
    auto e = *top();
    queue.pop();
    auto id = e.time_event_id;
    AWAIT_ALL(handle(std::move(e)));
    consume(id);
  }
}

void time_event_dispatcher::consume(std::int64_t event_id) {
  consumed_events.emplace(event_id);
}

}  // namespace bot

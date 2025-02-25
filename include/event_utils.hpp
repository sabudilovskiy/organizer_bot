#pragma once
#include <ranges>

#include "event.hpp"

namespace bot::events {

using namespace std::ranges::views;

template <EventType type>
constexpr auto is = std::ranges::views::filter(
    [](const Event& event) { return !event.consumed && (EventType)event.meta.index() == type; });

constexpr auto only_cb_queries = is<EventType::callback_query>;
constexpr auto only_commands = is<EventType::command>;
constexpr auto only_messages = is<EventType::message>;

inline bool should_main_menu(std::vector<Event>& events) {
  for (auto& e : events | only_commands | take(1)) {
    if (e.command_meta().text == "start") {
      e.consumed = true;
      return true;
    }
  }
  return false;
}

inline bool should_back(const std::vector<Event>& events) {
  for (auto& e : events | only_commands | take(1)) {
    return e.command_meta().text == "back";
  }
  return false;
}

}  // namespace bot::events

#pragma once
#include <ranges>

#include "io_event.hpp"

namespace bot::events {

using namespace std::ranges::views;

template <io_event_type type>
constexpr auto is = std::ranges::views::filter(
    [](const io_event& event) { return !event.consumed && (io_event_type)event.meta.index() == type; });

constexpr auto only_cb_queries = is<io_event_type::cb_query>;
constexpr auto only_commands = is<io_event_type::command>;
constexpr auto only_messages = is<io_event_type::message>;

inline bool should_main_menu(std::vector<io_event>& events) {
  for (auto& e : events | only_commands | take(1)) {
    if (e.command_meta().text == "start") {
      e.consumed = true;
      return true;
    }
  }
  return false;
}

inline bool should_back(const std::vector<io_event>& events) {
  for (auto& e : events | only_commands | take(1)) {
    return e.command_meta().text == "back";
  }
  return false;
}

}  // namespace bot::events

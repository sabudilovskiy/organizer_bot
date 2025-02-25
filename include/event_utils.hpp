#pragma once
#include <ranges>

#include "event.hpp"

namespace bot::events {

using namespace std::ranges::views;

template <EventType type>
constexpr auto is = std::ranges::views::filter(
    [](const Event& event) { return !event.consumed && (EventType)event.meta.index() == type; });

constexpr auto only_cb_queries = is<EventType::callback_query>;
constexpr auto only_messages = is<EventType::message>;

}  // namespace bot::events

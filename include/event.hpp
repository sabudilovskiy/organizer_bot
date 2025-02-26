#pragma once

#include "json/value.hpp"

namespace bot {

struct CBQueryMeta {
  std::string id;
  std::string data;
};

struct CommandMeta {
  std::string text;
};

struct MessageMeta {
  std::string text;
};

using EventMeta = std::variant<CBQueryMeta, CommandMeta, MessageMeta>;

template <typename T, typename Variant>
constexpr std::size_t variant_index() {
  static constexpr std::size_t N = std::variant_size_v<Variant>;
  return []<std::size_t... I>(std::index_sequence<I...>) {
    std::array<bool, N> eq{std::is_same_v<std::variant_alternative_t<I, Variant>, T>...};
    return std::distance(eq.begin(), std::find(eq.begin(), eq.end(), 1));
  }(std::make_index_sequence<N>{});
}

template <typename T>
constexpr std::size_t EventIndex() {
  return variant_index<T, EventMeta>();
}

enum struct EventType {
  callback_query = EventIndex<CBQueryMeta>(),
  command = EventIndex<CommandMeta>(),
  message = EventIndex<MessageMeta>(),
};

static_assert(magic_enum::enum_count<EventType>() == std::variant_size_v<EventMeta>);

struct Event {
  std::int64_t event_id;
  std::int64_t user_id;
  ts_t ts;
  EventMeta meta;
  bool consumed = false;

  static constexpr std::string_view db_name = "events";

  CBQueryMeta& cb_query_meta();
  const CBQueryMeta& cb_query_meta() const;

  CommandMeta& command_meta();
  const CommandMeta& command_meta() const;

  MessageMeta& message_meta();
  const MessageMeta& message_meta() const;
};

template <>
struct json_reader<EventMeta> {
  static EventMeta read(const boost::json::value& v);
};

template <>
struct json_writer<EventMeta> {
  static void write(boost::json::value& v, const EventMeta& meta);
};

}  // namespace bot

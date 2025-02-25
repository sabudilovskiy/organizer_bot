#pragma once

#include "json/value.hpp"

namespace bot {

enum struct EventType {
  message,
  callback_query,
};

struct MessageMeta {
  std::string text;
};

struct CBQueryMeta {
  std::string id;
  std::string data;
};

using EventMeta = std::variant<MessageMeta, CBQueryMeta>;
static_assert(magic_enum::enum_count<EventType>() == std::variant_size_v<EventMeta>);

struct Event {
  std::int64_t event_id;
  std::int64_t user_id;
  ts_t ts;
  bool consumed = false;
  EventMeta meta;

  MessageMeta& message_meta();
  const MessageMeta& message_meta() const;

  CBQueryMeta& cb_query_meta();
  const CBQueryMeta& cb_query_meta() const;
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

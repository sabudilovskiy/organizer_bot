#pragma once

#include "json/value.hpp"

namespace bot {

struct cb_query_meta_t {
  std::string id;
  std::string data;
};

struct command_meta_t {
  std::string text;
};

struct message_meta_t {
  std::string text;
};

using io_event_meta = std::variant<cb_query_meta_t, command_meta_t, message_meta_t>;

template <typename T, typename Variant>
constexpr std::size_t variant_index() {
  static constexpr std::size_t N = std::variant_size_v<Variant>;
  return []<std::size_t... I>(std::index_sequence<I...>) {
    std::array<bool, N> eq{std::is_same_v<std::variant_alternative_t<I, Variant>, T>...};
    return std::distance(eq.begin(), std::find(eq.begin(), eq.end(), 1));
  }(std::make_index_sequence<N>{});
}

template <typename T>
constexpr std::size_t IoEventIndex() {
  return variant_index<T, io_event_meta>();
}

enum struct io_event_type {
  callback_query = IoEventIndex<cb_query_meta_t>(),
  command = IoEventIndex<command_meta_t>(),
  message = IoEventIndex<message_meta_t>(),
};

static_assert(magic_enum::enum_count<io_event_type>() == std::variant_size_v<io_event_meta>);

struct io_event {
  std::int64_t io_event_id;
  std::int64_t user_id;
  ts_t ts;
  io_event_meta meta;
  bool consumed = false;

  static constexpr std::string_view db_name = "io_events";

  cb_query_meta_t& cb_query_meta();
  const cb_query_meta_t& cb_query_meta() const;

  command_meta_t& command_meta();
  const command_meta_t& command_meta() const;

  message_meta_t& message_meta();
  const message_meta_t& message_meta() const;
};

template <>
struct json_reader<io_event_meta> {
  static io_event_meta read(const boost::json::value& v);
};

template <>
struct json_writer<io_event_meta> {
  static void write(boost::json::value& v, const io_event_meta& meta);
};

}  // namespace bot

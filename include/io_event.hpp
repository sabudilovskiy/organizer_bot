#pragma once

#include "json/value.hpp"
#include "meta.hpp"

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

enum struct io_event_type {
  cb_query,
  command,
  message,
};

static_assert(magic_enum::enum_names<io_event_type>() == meta_names_v<io_event_meta>);

struct io_event {
  std::int64_t io_event_id;
  std::int64_t user_id;
  ts_t ts;
  io_event_meta meta;
  bool consumed = false;

  static constexpr std::string_view db_name = "io_events";
  using meta_type = io_event_type;

  io_event_type type() const;

  cb_query_meta_t& cb_query_meta();
  const cb_query_meta_t& cb_query_meta() const;

  command_meta_t& command_meta();
  const command_meta_t& command_meta() const;

  message_meta_t& message_meta();
  const message_meta_t& message_meta() const;
};

static_assert(aggregate_with_meta<io_event>);

}  // namespace bot

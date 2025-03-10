#include "io_event.hpp"

namespace bot {

const cb_query_meta_t& io_event::cb_query_meta() const {
  return std::get<cb_query_meta_t>(meta);
}

cb_query_meta_t& io_event::cb_query_meta() {
  return std::get<cb_query_meta_t>(meta);
}

const message_meta_t& io_event::message_meta() const {
  return std::get<message_meta_t>(meta);
}

message_meta_t& io_event::message_meta() {
  return std::get<message_meta_t>(meta);
}

const command_meta_t& io_event::command_meta() const {
  return std::get<command_meta_t>(meta);
}

command_meta_t& io_event::command_meta() {
  return std::get<command_meta_t>(meta);
}
}  // namespace bot

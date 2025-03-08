#include "io_event.hpp"

namespace bot {

io_event_meta json_reader<io_event_meta>::read(const boost::json::value& v) {
  io_event_meta result;
  auto& obj = v.as_object();
  io_event_type type = [&]() {
    auto it = obj.find("type");
    if (it == obj.end()) {
      throw std::runtime_error("not found");
    }
    auto e = magic_enum::enum_cast<io_event_type>(it->value().as_string());
    if (!e) {
      throw std::runtime_error("Unknown type");
    }
    return *e;
  }();

  tgbm::visit_index<magic_enum::enum_count<io_event_type>() - 1>(
      [&]<std::size_t I>() {
        using RType = std::variant_alternative_t<I, io_event_meta>;
        auto it = obj.find("value");
        if (it == obj.end()) {
          throw std::runtime_error("not found");
        }
        result = json_reader<RType>::read(it->value());
      },
      (std::size_t)type);
  return result;
}

void json_writer<io_event_meta>::write(boost::json::value& v, const io_event_meta& meta) {
  boost::json::object res;
  res["type"] = magic_enum::enum_name(io_event_type(meta.index()));
  std::visit([&]<typename V>(const V& value) { json_writer<V>::write(res["value"], value); }, meta);
  v = std::move(res);
}

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

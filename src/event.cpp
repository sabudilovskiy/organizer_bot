#include "event.hpp"

namespace bot {

EventMeta json_reader<EventMeta>::read(const boost::json::value& v) {
  EventMeta result;
  auto& obj = v.as_object();
  EventType type = [&]() {
    auto it = obj.find("type");
    if (it == obj.end()) {
      throw std::runtime_error("not found");
    }
    auto e = magic_enum::enum_cast<EventType>(it->value().as_string());
    if (!e) {
      throw std::runtime_error("Unknown type");
    }
    return *e;
  }();

  tgbm::visit_index<magic_enum::enum_count<EventType>() - 1>(
      [&]<std::size_t I>() {
        using RType = std::variant_alternative_t<I, EventMeta>;
        auto it = obj.find("value");
        if (it == obj.end()) {
          throw std::runtime_error("not found");
        }
        result = json_reader<RType>::read(it->value());
      },
      (std::size_t)type);
  return result;
}

void json_writer<EventMeta>::write(boost::json::value& v, const EventMeta& meta) {
  boost::json::object res;
  res["type"] = magic_enum::enum_name(EventType(meta.index()));
  std::visit([&]<typename V>(const V& value) { json_writer<V>::write(res["value"], value); }, meta);
  v = std::move(res);
}

const CBQueryMeta& Event::cb_query_meta() const {
  return std::get<CBQueryMeta>(meta);
}

CBQueryMeta& Event::cb_query_meta() {
  return std::get<CBQueryMeta>(meta);
}

const MessageMeta& Event::message_meta() const {
  return std::get<MessageMeta>(meta);
}

MessageMeta& Event::message_meta() {
  return std::get<MessageMeta>(meta);
}

}  // namespace bot

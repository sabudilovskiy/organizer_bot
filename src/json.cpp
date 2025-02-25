#include "json/value.hpp"

namespace bot {

std::string json_reader<std::string>::read(const boost::json::value& v) {
  auto& str = v.as_string();
  return std::string(str.data(), str.size());
}

std::int64_t json_reader<std::int64_t>::read(const boost::json::value& v) {
  return v.as_int64();
}

bool json_reader<bool>::read(const boost::json::value& v) {
  return v.as_bool();
}

ts_t json_reader<ts_t>::read(const boost::json::value& v) {
  return parse_ts(json_reader<std::string>::read(v));
}

boost::json::object json_reader<boost::json::object>::read(const boost::json::value& v) {
  return v.as_object();
}

void json_writer<std::string>::write(boost::json::value& v, const std::string& str) {
  v = str;
}

void json_writer<int64_t>::write(boost::json::value& v, int64_t i) {
  v = i;
}

void json_writer<ts_t>::write(boost::json::value& v, ts_t ts) {
  v = to_string(ts);
}

json_view json_value::operator[](std::size_t idx) {
  return json_view{value.as_array()[idx]};
}

json_view json_value::operator[](std::string_view key) {
  return json_view{value.as_object()[key]};
}

const_json_view json_value::operator[](std::size_t idx) const {
  return const_json_view{value.as_array()[idx]};
}

const_json_view json_value::operator[](std::string_view key) const {
  auto it = value.as_object().find(key);
  if (it != value.as_object().end()) {
    throw std::runtime_error("not found");
  }
  return const_json_view{it->value()};
}

json_value json_reader<json_value>::read(const boost::json::value& v) {
  return json_value(v);
}

void json_writer<json_value>::write(boost::json::value& v, const json_value& j) {
  v = j.value;
}

json_value json_value::object() noexcept {
  json_value res;
  res.value = boost::json::object{};
  return res;
}

json_value json_value::array() noexcept {
  json_value res;
  res.value = boost::json::array{};
  return res;
}

void json_writer<bool>::write(boost::json::value& v, bool b) {
  v = b;
}

}  // namespace bot

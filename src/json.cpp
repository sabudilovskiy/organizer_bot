#include "json/value.hpp"

namespace bot {

namespace {

std::string serialize_value(const boost::json::value& jv, std::string_view indent,
                            int quantity);

std::string serialize_object(const boost::json::object& obj, std::string_view indent,
                             int quantity) {
  if (obj.empty())
    return "{}";

  std::string result = "{\n";
  for (auto it = obj.begin(); it != obj.end(); ++it) {
    result.append(indent.data(), indent.size() * (quantity + 1));
    result += boost::json::serialize(it->key()) + " : " +
              serialize_value(it->value(), indent, quantity + 1);
    if (std::next(it) != obj.end())
      result += ",\n";
  }
  result += "\n";
  result.append(indent.data(), indent.size() * quantity);
  result += "}";
  return result;
}

std::string serialize_array(const boost::json::array& arr, std::string_view indent,
                            int quantity) {
  if (arr.empty())
    return "[]";

  std::string result = "[\n";
  for (auto it = arr.begin(); it != arr.end(); ++it) {
    result.append(indent.data(), indent.size() * (quantity + 1));
    result += serialize_value(*it, indent, quantity + 1);
    if (std::next(it) != arr.end())
      result += ",\n";
  }
  result += "\n";
  result.append(indent.data(), indent.size() * quantity);
  result += "]";
  return result;
}

std::string serialize_value(const boost::json::value& jv, std::string_view indent,
                            int quantity) {
  switch (jv.kind()) {
    case boost::json::kind::object:
      return serialize_object(jv.get_object(), indent, quantity);
    case boost::json::kind::array:
      return serialize_array(jv.get_array(), indent, quantity);
    case boost::json::kind::string:
      return boost::json::serialize(jv.get_string());
    case boost::json::kind::uint64:
    case boost::json::kind::int64:
    case boost::json::kind::double_:
      return boost::json::serialize(jv);
    case boost::json::kind::bool_:
      return jv.get_bool() ? "true" : "false";
    case boost::json::kind::null:
      return "null";
  }
  return "";
}

}  // namespace

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

std::string pretty_serialize(const boost::json::value& value, std::string_view indent) {
  return serialize_value(value, indent, 0);
}

}  // namespace bot

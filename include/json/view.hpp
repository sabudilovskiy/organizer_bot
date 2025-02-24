#pragma once

#include "json/reader.hpp"
#include "json/writer.hpp"

namespace bot {
struct const_json_view {
  const boost::json::value& value;

  template <typename T>
  operator T() const {
    return json_reader<T>::read(value);
  }

  template <typename T>
  T as() const {
    return json_reader<T>::read(value);
  }
};

struct json_view {
  boost::json::value& value;

  template <typename T>
  operator T() const {
    return json_reader<T>::read(value);
  }

  template <typename T>
  json_view& operator=(const T& t) {
    json_writer<T>::write(value, t);
    return *this;
  }

  template <typename T>
  T as() const {
    return json_reader<T>::read(value);
  }

  std::string serialize() const {
    return boost::json::serialize(value);
  }
};
}  // namespace bot

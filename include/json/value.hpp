#pragma once

#include <boost/json.hpp>
#include <magic_enum/magic_enum.hpp>
#include <tgbm/api/optional.hpp>
#include <tgbm/utils/pfr_extension.hpp>

#include "json/view.hpp"
#include "time.hpp"

namespace bot {

struct json_value;

template <>
struct json_reader<json_value> {
  static json_value read(const boost::json::value& v);
};

template <>
struct json_writer<json_value> {
  static void write(boost::json::value& v, const json_value& j);
};

struct json_value {
  boost::json::value value;

  static json_value object() noexcept;

  static json_value array() noexcept;

  json_value() = default;

  json_value(boost::json::value v) noexcept : value(std::move(v)) {
  }

  json_value(boost::json::object obj) noexcept : value(std::move(obj)) {
  }

  json_value(const json_value& j) noexcept : value(j.value) {
  }
  json_value(json_value&& j) noexcept {
    *this = std::move(j);
  }

  template <typename T>
  json_value(const T& t) {
    json_writer<T>::write(value, t);
  }

  std::string serialize() const {
    return boost::json::serialize(value);
  }

  template <typename T>
  operator T() const {
    return json_reader<T>::read(value);
  }

  template <typename T>
  json_value& operator=(const T& t) {
    json_writer<T>::write(value, t);
    return *this;
  }

  json_value& operator=(json_value&& j) noexcept {
    std::swap(j.value, value);
    return *this;
  }

  json_view operator[](std::size_t idx);

  template <std::size_t N>
  json_view operator[](const char (&arr)[N]) {
    return (*this)[std::string_view{arr}];
  }

  json_view operator[](std::string_view key);

  template <std::size_t N>
  const_json_view operator[](const char (&arr)[N]) const {
    return (*this)[std::string_view{arr}];
  }

  const_json_view operator[](std::size_t idx) const;

  const_json_view operator[](std::string_view key) const;

  template <typename T>
  T as() {
    T res;
    res = json_view{value};
    return res;
  }
};

template <typename T>
std::string to_json_str(const T& t) {
  return json_value(t).serialize();
}

template <typename T>
T from_json_str(std::string_view j) {
  boost::json::value v(j);
  return json_view(v).as<T>();
}

}  // namespace bot

#pragma once

#include <boost/json.hpp>
#include <magic_enum/magic_enum.hpp>
#include <tgbm/api/optional.hpp>
#include <tgbm/utils/pfr_extension.hpp>

#include "time.hpp"

namespace bot {

template <typename T>
struct json_reader {};

template <>
struct json_reader<std::string> {
  static std::string read(const boost::json::value& v);
};

template <>
struct json_reader<std::int64_t> {
  static std::int64_t read(const boost::json::value& v);
};

template <>
struct json_reader<bool> {
  static bool read(const boost::json::value& v);
};

template <>
struct json_reader<ts_t> {
  static ts_t read(const boost::json::value& v);
};

template <>
struct json_reader<boost::json::object> {
  static boost::json::object read(const boost::json::value& v);
};

template <typename T>
  requires magic_enum::is_scoped_enum_v<T>
struct json_reader<T> {
  static T read(const boost::json::value& v) {
    auto& str = v.as_string();
    std::string_view sv(str.data(), str.size());
    auto res = magic_enum::enum_cast<T>(sv);
    if (!res) {
      throw std::runtime_error("Unknown value");
    }
    return *res;
  }
};

template <typename T>
  requires std::is_aggregate_v<T>
struct json_reader<T> {
  static T read(const boost::json::value& v) {
    T res;
    pfr_extension::visit_object(res, [&]<typename Info, typename F>(F& field) {
      auto& obj = v.as_object();
      std::string_view key = Info::name.AsStringView();
      auto it = obj.find(key);
      if (it == obj.end()) {
        throw std::runtime_error("not found");
      }
      field = json_reader<F>(it->value());
    });
    return res;
  }
};

template <typename T>
struct json_reader<tgbm::api::optional<T>> {
  static tgbm::api::optional<T> read(const boost::json::value& v) {
    if (v.is_null()) {
      return std::nullopt;
    }
    return json_reader<T>::read(v);
  }
};

template <typename T>
struct json_writer {};

template <>
struct json_writer<std::string> {
  static void write(boost::json::value& v, const std::string& str);
};

template <>
struct json_writer<int64_t> {
  static void write(boost::json::value& v, int64_t i);
};

template <>
struct json_writer<ts_t> {
  static void write(boost::json::value& v, ts_t ts);
};

template <typename T>
  requires magic_enum::is_scoped_enum_v<T>
struct json_writer<T> {
  static void write(boost::json::value& v, T t) {
    v = magic_enum::enum_name(t);
  }
};

template <typename T>
struct json_writer<tgbm::api::optional<T>> {
  static void write(boost::json::value& v, const tgbm::api::optional<T>& t) {
    if (t.has_value()) {
      json_writer<T>::write(v, t.value());
    } else
      v = boost::json::value{};
  }
};

template <typename T>
  requires std::is_aggregate_v<T>
struct json_writer<T> {
  static void write(boost::json::value& v, const T& obj) {
    v = boost::json::object{};
    auto& j_o = v.as_object();
    pfr_extension::visit_object(obj, [&]<typename Info, typename F>(const F& field) {
      std::string_view key = Info::name.AsStringView();
      json_writer<F>(j_o[key], field);
    });
  }
};

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
};

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

  template <typename T>
  operator T() const {
    return json_reader<T>::read(value);
  }

  template <typename T>
  json_value& operator=(const T& t) {
    json_writer<T>::write(value, t);
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

}  // namespace bot

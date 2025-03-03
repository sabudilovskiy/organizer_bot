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
    auto& obj = v.as_object();
    pfr_extension::visit_object(res, [&]<typename Info, typename F>(F& field) {
      std::string_view key = Info::name.AsStringView();
      auto it = obj.find(key);
      if (it == obj.end()) {
        throw std::runtime_error("not found");
      }
      field = json_reader<F>::read(it->value());
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

template <>
struct json_reader<weekday> {
  static weekday read(const boost::json::value& v) {
    auto str = json_reader<std::string>::read(v);
    return parse_weekday(str);
  }
};

}  // namespace bot

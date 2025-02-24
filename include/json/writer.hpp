#pragma once
#include <boost/json.hpp>
#include <magic_enum/magic_enum.hpp>
#include <tgbm/api/optional.hpp>
#include <tgbm/utils/pfr_extension.hpp>

#include "time.hpp"

namespace bot {
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
      json_writer<F>::write(j_o[key], field);
    });
  }
};
}  // namespace bot

#pragma once
#include <boost/json.hpp>
#include <magic_enum/magic_enum.hpp>
#include <tgbm/api/optional.hpp>
#include <tgbm/utils/pfr_extension.hpp>

#include "meta.hpp"
#include "traits.hpp"

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
struct json_writer<bool> {
  static void write(boost::json::value& v, bool b);
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
  requires tgbm::aggregate<T>
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

template <typename T>
  requires aggregate_with_meta<T>
struct json_writer<T> {
  static void write(boost::json::value& v, const T& obj) {
    v = boost::json::object{};
    auto& j_o = v.as_object();
    visit_object_with_meta(obj, [&]<typename Info, typename F>(const F& field) {
      std::string_view key = Info::name.AsStringView();
      json_writer<F>::write(j_o[key], field);
    });
  }
};

template <typename... Ts>
struct json_writer<std::variant<Ts...>> {
  static void write(boost::json::value& j, const std::variant<Ts...>& v) {
    std::visit([&]<typename U>(const U& u) { json_writer<U>::write(j, u); }, v);
  }
};

template <std::ranges::range R>
struct json_writer<R> {
  static void write(boost::json::value& j, const R& r) {
    using T = std::ranges::range_value_t<R>;
    j = boost::json::array{};
    auto& j_a = j.as_array();
    for (const T& t : r) {
      json_writer<T>::write(j_a.emplace_back(boost::json::value{}), t);
    }
  }
};

template <serializable T>
struct json_writer<T> {
  static void write(boost::json::value& v, const T& t) {
    v = t.serialize();
  }
};

}  // namespace bot

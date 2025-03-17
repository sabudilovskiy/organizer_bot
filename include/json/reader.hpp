#pragma once
#include <boost/json.hpp>
#include <magic_enum/magic_enum.hpp>
#include <set>
#include <tgbm/api/optional.hpp>
#include <tgbm/utils/pfr_extension.hpp>

#include "meta.hpp"
#include "traits.hpp"
#include "utils.hpp"

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

template <parseable T>
struct json_reader<T> {
  static T read(const boost::json::value& v) {
    return T::parse(v.as_string());
  }
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
  requires tgbm::aggregate<T>
struct json_reader<T> {
  static T read(const boost::json::value& v) {
    T res;
    auto& obj = v.as_object();
    pfr_extension::visit_object(res, [&]<typename Info, typename F>(F& field) {
      std::string_view key = Info::name.AsStringView();
      auto it = obj.find(key);
      field = json_reader<F>::read(it != obj.end() ? it->value() : boost::json::value{});
    });
    return res;
  }
};

template <typename T>
  requires aggregate_with_meta<T>
struct json_reader<T> {
  static T read(const boost::json::value& v) {
    T out;
    auto& obj = v.as_object();
    auto it = obj.find("meta_type");
    if (it == obj.end()) {
      throw std::runtime_error("not found `meta_type`");
    }
    auto meta_type = magic_enum::enum_cast<meta_type_t<T>>(it->value().as_string());
    if (!meta_type) {
      throw std::runtime_error("unknown meta_type value");
    }
    emplace_meta(*meta_type, out);

    visit_object_with_meta(out, [&]<typename Info, typename F>(F& field) {
      std::string_view key = Info::name.AsStringView();
      auto it = obj.find(key);
      field = json_reader<F>::read(it != obj.end() ? it->value() : boost::json::value{});
    });
    return out;
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
struct json_reader<std::vector<T>> {
  static std::vector<T> read(const boost::json::value& v) {
    std::vector<T> out;
    auto& arr = v.as_array();
    for (auto& e : arr) {
      out.emplace_back(json_reader<T>::read(e));
    }
    return out;
  }
};

template <typename T>
struct json_reader<std::set<T>> {
  static std::set<T> read(const boost::json::value& v) {
    std::set<T> out;
    auto& arr = v.as_array();
    for (auto& e : arr) {
      out.emplace(json_reader<T>::read(e));
    }
    return out;
  }
};

}  // namespace bot

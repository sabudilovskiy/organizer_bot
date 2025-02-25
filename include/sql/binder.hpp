#pragma once

#include <SQLiteCpp/Statement.h>
#include <tgbm/api/optional.hpp>

#include "event.hpp"
#include "json/value.hpp"
#include "time.hpp"

namespace bot::sql {
template <typename T>
struct binder {};

template <>
struct binder<std::string> {
  static void bind(SQLite::Statement& statement, const std::string& arg, std::size_t& cur_index) {
    statement.bindNoCopy(cur_index++, arg);
  }
};

template <>
struct binder<int64_t> {
  static void bind(SQLite::Statement& statement, int64_t arg, std::size_t& cur_index) {
    statement.bind(cur_index++, arg);
  }
};

template <>
struct binder<bool> {
  static void bind(SQLite::Statement& statement, bool arg, std::size_t& cur_index) {
    statement.bind(cur_index++, arg);
  }
};

template <>
struct binder<json_value> {
  static void bind(SQLite::Statement& statement, const json_value& arg, std::size_t& cur_index) {
    statement.bind(cur_index++, serialize(arg.value));
  }
};

template <typename T>
struct binder<tgbm::api::optional<T>> {
  static void bind(SQLite::Statement& statement, const tgbm::api::optional<T>& arg, std::size_t& cur_index) {
    if (!arg) {
      statement.bind(cur_index++);
    } else {
      binder<T>::bind(statement, arg.value(), cur_index);
    }
  }
};

template <>
struct binder<ts_t> {
  static void bind(SQLite::Statement& statement, const ts_t& arg, std::size_t& cur_index) {
    auto str = to_string(arg);
    statement.bind(cur_index++, str);
  }
};

template <typename T>
  requires std::is_aggregate_v<T>
struct binder<T> {
  static void bind(SQLite::Statement& statement, const T& arg, std::size_t& cur_index) {
    auto v = [&]<typename Field>(const Field& field, std::size_t i) {
      binder<Field>::bind(statement, field, cur_index);
    };
    boost::pfr::for_each_field(arg, v);
  }
};

template <typename T>
  requires magic_enum::is_scoped_enum_v<T>
struct binder<T> {
  static void bind(SQLite::Statement& statement, T arg, std::size_t& cur_index) {
    std::string_view str = magic_enum::enum_name(arg);
    statement.bind(cur_index++, std::string(str));
  }
};

template <>
struct binder<EventMeta> {
  static void bind(SQLite::Statement& statement, const EventMeta& meta, std::size_t& cur_index) {
    statement.bind(cur_index++, to_json_str(meta));
  }
};
}  // namespace bot::sql

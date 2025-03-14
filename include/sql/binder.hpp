#pragma once

#include <SQLiteCpp/Statement.h>
#include <tgbm/api/optional.hpp>

#include "io_event.hpp"
#include "json/value.hpp"
#include "time.hpp"

namespace bot::sql {

template <typename T>
struct as_sequence {
  const T& t;
};

template <typename T>
struct binder {};

template <>
struct binder<std::string> {
  static void bind(SQLite::Statement& statement, const std::string& arg,
                   std::size_t& cur_index) {
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
  static void bind(SQLite::Statement& statement, const json_value& arg,
                   std::size_t& cur_index) {
    statement.bind(cur_index++, serialize(arg.value));
  }
};

template <typename T>
struct binder<tgbm::api::optional<T>> {
  static void bind(SQLite::Statement& statement, const tgbm::api::optional<T>& arg,
                   std::size_t& cur_index) {
    if (!arg) {
      statement.bind(cur_index++);
    } else {
      binder<T>::bind(statement, arg.value(), cur_index);
    }
  }
};

template <>
struct binder<ts_t> {
  static void bind(SQLite::Statement& statement, const ts_t& arg,
                   std::size_t& cur_index) {
    auto str = to_string(arg);
    statement.bind(cur_index++, str);
  }
};

template <typename T>
  requires tgbm::aggregate<T>
struct binder<as_sequence<T>> {
  static void bind(SQLite::Statement& statement, const as_sequence<T>& arg,
                   std::size_t& cur_index) {
    auto v = [&]<typename Field>(const Field& field) mutable {
      binder<Field>::bind(statement, field, cur_index);
    };
    boost::pfr::for_each_field(arg.t, v);
  }
};

template <typename T>
  requires aggregate_with_meta<T>
struct binder<as_sequence<T>> {
  static void bind(SQLite::Statement& statement, const as_sequence<T>& arg,
                   std::size_t& cur_index) {
    auto v = [&]<typename Info, typename Field>(const Field& field) mutable {
      binder<Field>::bind(statement, field, cur_index);
    };
    visit_object_with_meta(arg.t, v);
  }
};

template <typename T>
struct binder<as_sequence<std::vector<T>>> {
  static void bind(SQLite::Statement& statement, const as_sequence<std::vector<T>>& arg,
                   std::size_t& cur_index) {
    for (auto& t : arg.t) {
      binder<T>::bind(statement, t, cur_index);
    }
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

template <typename T>
  requires std::is_aggregate_v<T>
struct binder<T> {
  static void bind(SQLite::Statement& statement, const T& t, std::size_t& cur_index) {
    statement.bind(cur_index++, to_json_str(t));
  }
};

template <typename... Ts>
struct binder<std::variant<Ts...>> {
  static void bind(SQLite::Statement& statement, const std::variant<Ts...>& t,
                   std::size_t& cur_index) {
    statement.bind(cur_index++, to_json_str(t));
  }
};

template <>
struct binder<std::chrono::weekday> {
  static void bind(SQLite::Statement& statement, const std::chrono::weekday& w,
                   std::size_t& cur_index) {
    statement.bind(cur_index++, std::string(to_string(w)));
  }
};
}  // namespace bot::sql

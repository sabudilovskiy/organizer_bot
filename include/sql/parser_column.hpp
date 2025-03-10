#pragma once

#include <SQLiteCpp/Statement.h>

#include "io_event.hpp"
#include "json/value.hpp"
#include "sql/native_type.hpp"
#include "time_event.hpp"

namespace bot::sql {

template <typename T>
struct parser_column {
  // static constexpr auto nt = native_type::integer;
  // static constexpr auto is_null = true;
};

template <>
struct parser_column<bool> {
  static constexpr auto nt = native_type::INTEGER;
  static constexpr auto is_null = false;
  static bool parse(SQLite::Statement& statement, std::size_t index);
};

template <>
struct parser_column<int64_t> {
  static constexpr auto nt = native_type::INTEGER;
  static constexpr auto is_null = false;
  static int64_t parse(SQLite::Statement& statement, std::size_t index);
};

template <>
struct parser_column<json_value> {
  static constexpr auto nt = native_type::TEXT;
  static constexpr auto is_null = false;
  static json_value parse(SQLite::Statement& statement, std::size_t index);
};

template <>
struct parser_column<std::string> {
  static constexpr auto nt = native_type::TEXT;
  static constexpr auto is_null = false;
  static std::string parse(SQLite::Statement& statement, std::size_t index);
};

template <>
struct parser_column<io_event_meta> {
  static constexpr auto nt = native_type::TEXT;
  static constexpr auto is_null = false;
  static io_event_meta parse(SQLite::Statement& statement, std::size_t index);
};

template <>
struct parser_column<time_event_meta> {
  static constexpr auto nt = native_type::TEXT;
  static constexpr auto is_null = false;
  static time_event_meta parse(SQLite::Statement& statement, std::size_t index);
};

template <typename T>
  requires std::is_aggregate_v<T>
struct parser_column<T> {
  static constexpr auto nt = native_type::TEXT;
  static constexpr auto is_null = false;

  static T parse(SQLite::Statement& statement, std::size_t index) {
    return from_json_str<T>(parser_column<std::string>::parse(statement, index));
  }
};

template <typename T>
  requires magic_enum::is_scoped_enum_v<T>
struct parser_column<T> {
  static constexpr auto nt = native_type::TEXT;
  static constexpr auto is_null = false;
  static T parse(SQLite::Statement& statement, std::size_t index) {
    auto str = parser_column<std::string>::parse(statement, index);
    auto r = magic_enum::enum_cast<T>(str);
    if (!r) {
      throw std::runtime_error("unknown enum");
    }
    return *r;
  }
};

template <>
struct parser_column<ts_t> {
  static constexpr auto nt = native_type::TEXT;
  static constexpr auto is_null = false;
  static ts_t parse(SQLite::Statement& statement, std::size_t index);
};

template <typename T>
struct parser_column<tgbm::api::optional<T>> {
  static constexpr auto nt = parser_column<T>::nt;
  static constexpr auto is_null = true;
  static tgbm::api::optional<T> parse(SQLite::Statement& statement, std::size_t index) {
    if (statement.getColumnCount() <= index) {
      throw std::runtime_error("too big index");
    }
    if (statement.isColumnNull(index)) {
      return std::nullopt;
    }
    return parser_column<T>::parse(statement, index);
  }
};

template <typename T>
struct parser_column<std::vector<T>> {
  static constexpr auto nt = native_type::TEXT;
  static constexpr auto is_null = false;
  static tgbm::api::optional<T> parse(SQLite::Statement& statement, std::size_t index) {
    if (statement.getColumnCount() <= index) {
      throw std::runtime_error("too big index");
    }
    return from_json_str<T>(parser_column<std::string>::parse(statement, index));
  }
};

}  // namespace bot::sql

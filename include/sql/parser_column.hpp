#pragma once

#include <SQLiteCpp/Statement.h>

#include "json/value.hpp"

namespace bot::sql {

template <typename T>
struct parser_column {};

template <>
struct parser_column<bool> {
  static bool parse(SQLite::Statement& statement, std::size_t index);
};

template <>
struct parser_column<int64_t> {
  static int64_t parse(SQLite::Statement& statement, std::size_t index);
};

template <>
struct parser_column<json_value> {
  static json_value parse(SQLite::Statement& statement, std::size_t index);
};

template <>
struct parser_column<std::string> {
  static std::string parse(SQLite::Statement& statement, std::size_t index);
};

template <typename T>
  requires magic_enum::is_scoped_enum_v<T>
struct parser_column<T> {
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
  static ts_t parse(SQLite::Statement& statement, std::size_t index);
};

template <typename T>
struct parser_column<tgbm::api::optional<T>> {
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

}  // namespace bot::sql

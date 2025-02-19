#pragma once

#include "sql/parser_row.hpp"

namespace bot::sql {

template <typename T>
struct parser {
  static T parse(SQLite::Statement& statement) {
    if (!statement.executeStep()) {
      throw std::runtime_error("empty result");
    }
    auto res = parser_row<T>::parse(statement);
    if (statement.executeStep()) {
      throw std::runtime_error("too much results");
    }
    return res;
  }
};

template <typename T>
struct parser<tgbm::api::optional<T>> {
  static tgbm::api::optional<T> parse(SQLite::Statement& statement) {
    if (!statement.executeStep()) {
      return std::nullopt;
    }
    tgbm::api::optional<T> res = parser_row<T>::parse(statement);
    if (statement.executeStep()) {
      throw std::runtime_error("too much results");
    }
    return res;
  }
};

template <typename T>
struct parser<std::vector<T>> {
  static std::vector<T> parse(SQLite::Statement& statement) {
    std::vector<T> out;
    while (statement.executeStep()) {
      out.emplace_back(parser_row<T>::parse(statement));
    }
    return out;
  }
};

template <>
struct parser<void> {
  static void parse(SQLite::Statement& statement) {
    while (statement.executeStep()) {
    }
  }
};

}  // namespace bot::sql

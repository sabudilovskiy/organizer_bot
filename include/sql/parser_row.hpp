#pragma once

#include <boost/pfr.hpp>

#include "sql/parser_column.hpp"

namespace bot::sql {
template <typename T>
struct parser_row {
  static T parse(SQLite::Statement& statement) {
    T out;
    if (statement.getColumnCount() != 1) {
      throw std::runtime_error("Unexpected size");
    }
    parser_column<T>::parse(statement, 0);
    return out;
  }
};

template <typename T>
  requires std::is_aggregate_v<T>
struct parser_row<T> {
  static T parse(SQLite::Statement& statement) {
    T out;
    auto v = [&]<typename Field>(Field& field, std::size_t i) {
      field = parser_column<Field>::parse(statement, i);
    };
    boost::pfr::for_each_field(out, v);
    return out;
  }
};

}  // namespace bot::sql

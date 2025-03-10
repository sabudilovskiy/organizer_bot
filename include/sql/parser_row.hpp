#pragma once

#include <boost/pfr.hpp>

#include "sql/parser_column.hpp"
#include "utils.hpp"

#include <tgbm_replace/logger.hpp>

namespace bot::sql {
template <typename T>
struct parser_row {
  static T parse(SQLite::Statement& statement) {
    if (statement.getColumnCount() != 1) {
      throw std::runtime_error("Unexpected size");
    }
    return parser_column<T>::parse(statement, 0);
  }
};

template <typename T>
  requires std::is_aggregate_v<T>
struct parser_row<T> {
  static T parse(SQLite::Statement& statement) {
    T out;
    auto v = [&]<typename Info, typename Field>(Field& field) {
      TGBM_ON_DEBUG({
        std::string_view got_name = statement.getColumnName(Info::index);
        std::string_view expected_name = Info::name.AsStringView();
        if (got_name != expected_name) {
          throw_formatted("[T = {}] Missmatch field name `{}` and column name `{}` detected", name_type_v<T>,
                          got_name, expected_name);
        }
      });
      field = parser_column<Field>::parse(statement, Info::index);
    };
    pfr_extension::visit_object(out, v);
    return out;
  }
};

}  // namespace bot::sql

#pragma once

#include <boost/pfr.hpp>

#include "formatters/all.hpp"
#include "sql/parser_column.hpp"
#include "utils.hpp"

#include <tgbm_replace/logger.hpp>

namespace bot::sql {

namespace details {

inline void check_column_type_erased(const std::string& query,
                                     const SQLite::Column& column, std::size_t index,
                                     std::string_view type_name,
                                     native_type expected_column_type, bool is_nullable) {
  std::string_view got_name = column.getName();
  if (!is_nullable && column.isNull()) {
    throw_formatted(
        "Error while executing query: [{}]. "
        "C++ Type: {}"
        "Unexpected null value got from column `{}` with index {}",
        query, type_name, got_name, index);
  }
  if (is_nullable && column.isNull()) {
    return;
  }
  auto got_type = from_sqlite_int(column.getType());

  if (expected_column_type != got_type) {
    throw_formatted(
        "Error while executing query: [{}]. "
        "C++ Type: {}"
        "Missmatch names column `{}` with index {} detected: "
        " got type: `{}`, expected type: `{}`",
        query, type_name, got_name, index, got_type, expected_column_type);
  }
}

template <typename T>
inline void check_column_type(const std::string& query, const SQLite::Column& column,
                              std::size_t index) {
  check_column_type_erased(query, column, index, name_type_v<T>, parser_column<T>::nt,
                           parser_column<T>::is_null);
}

inline void check_column_name_erased(const std::string& query,
                                     const SQLite::Column& column, std::size_t index,
                                     std::string_view expected_column_name,
                                     std::string_view type_name) {
  std::string_view got_name = column.getName();

  if (expected_column_name != got_name) {
    throw_formatted(
        "Error while executing query: [{}]. "
        "C++ Type: {}"
        " Missmatch names column with index {} detected: "
        " got name: `{}`, expected name: `{}`",
        query, type_name, index, got_name, expected_column_name);
  }
}

template <typename T>
inline void check_column_name(const std::string& query, const SQLite::Column& column,
                              std::size_t index, std::string_view expected_column_name) {
  check_column_name_erased(query, column, index, expected_column_name, name_type_v<T>);
}

template <typename T>
inline void check_column_all(const std::string& query, const SQLite::Column& column,
                             std::size_t index, std::string_view expected_column_name) {
  check_column_type<T>(query, column, index);
  check_column_name<T>(query, column, index, expected_column_name);
}

inline void check_column_all_erased(const std::string& query,
                                    const SQLite::Column& column, std::size_t index,
                                    std::string_view type_name,
                                    std::string_view expected_column_name,
                                    native_type expected_column_type, bool is_nullable) {
  check_column_type_erased(query, column, index, type_name, expected_column_type,
                           is_nullable);
  check_column_name_erased(query, column, index, expected_column_name, type_name);
}

}  // namespace details

template <typename T>
struct parser_row {
  static T parse(SQLite::Statement& statement) {
    T out;
    if (statement.getColumnCount() != 1) {
      throw std::runtime_error("Unexpected size");
    }
    auto col = statement.getColumn(0);
    TGBM_ON_DEBUG(details::check_column_type<T>(statement.getQuery(), col, 0));
    bool ok = parser_column<T>::parse(col, out);
    if (!ok) {
      throw_formatted("Fail parse `{}`", name_type_v<T>);
    }
    return out;
  }
};

template <typename T>
  requires tgbm::aggregate<T>
struct parser_row<T> {
  static T parse(SQLite::Statement& statement) {
    T out;
    auto v = [&]<typename Info, typename Field>(Field& out) {
      std::size_t index = Info::index;
      std::string_view name = Info::name.AsStringView();
      auto col = statement.getColumn(index);
      TGBM_ON_DEBUG(
          details::check_column_all<Field>(statement.getQuery(), col, index, name));
      if (!parser_column<Field>::parse(col, out)) {
        throw_formatted("Fail parse `{}` in `{}`", name, name_type_v<T>);
      }
    };
    pfr_extension::visit_object(out, v);
    return out;
  }
};

template <typename T>
  requires aggregate_with_meta<T>
struct parser_row<T> {
  static T parse(SQLite::Statement& statement) {
    T out;
    std::size_t meta_type_index = pfr_extension::tuple_size_v<T>;
    auto meta_type_col = statement.getColumn(meta_type_index);
    TGBM_ON_DEBUG(details::check_column_all<meta_type_t<T>>(
        statement.getQuery(), meta_type_col, meta_type_index, "meta_type"));

    meta_type_t<T> meta_type;
    if (!parser_column<meta_type_t<T>>::parse(meta_type_col, meta_type)) {
      throw_formatted("Fail parse `{}` in `{}`", "meta_type", name_type_v<T>);
    }
    emplace_meta<T>(meta_type, out);

    auto v = [&]<typename Info, typename Field>(Field& out) {
      std::size_t index = Info::index;
      std::string_view name = Info::name.AsStringView();
      auto col = statement.getColumn(index);
      TGBM_ON_DEBUG(
          details::check_column_all<Field>(statement.getQuery(), col, index, name));
      if (!parser_column<Field>::parse(col, out)) {
        throw_formatted("Fail parse `{}` in `{}`", name, name_type_v<T>);
      }
    };
    visit_object_with_meta(out, v);
    return out;
  }
};

}  // namespace bot::sql

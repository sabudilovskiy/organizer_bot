#pragma once

#include <SQLiteCpp/Column.h>
#include <tgbm_replace/logger.hpp>

#include "io_event.hpp"
#include "json/value.hpp"
#include "sql/native_type.hpp"
#include "time_event.hpp"

namespace bot::sql {

template <typename T>
struct parser_column {
  // static constexpr auto nt = native_type::integer;
  // static constexpr auto is_null = true;
  // static bool parse(const SQLite::Column& column, T& out);
};

#define parser_column_decl(TYPE, NT_TYPE, IS_NULL)              \
  template <>                                                   \
  struct parser_column<TYPE> {                                  \
    static constexpr auto nt = native_type::NT_TYPE;            \
    static constexpr auto is_null = IS_NULL;                    \
    static bool parse(const SQLite::Column& column, TYPE& out); \
  };

parser_column_decl(bool, integer, false);
parser_column_decl(int64_t, integer, false);
parser_column_decl(std::string, text, false);
parser_column_decl(json_value, text, false);
parser_column_decl(io_event_meta, text, false);
parser_column_decl(native_type, text, false);
parser_column_decl(time_event_meta, text, false);
parser_column_decl(ts_t, text, false);

#undef parser_column_decl

template <typename T>
  requires std::is_aggregate_v<T>
struct parser_column<T> {
  static constexpr auto nt = native_type::text;
  static constexpr auto is_null = false;

  static bool parse(const SQLite::Column& column, T& out) {
    std::string str;
    bool ok = parser_column<std::string>::parse(column, str);
    if (!ok) {
      return false;
    }
    out = from_json_str<T>(str);
    return true;
  }
};

template <typename T>
struct parser_column<std::vector<T>> {
  static constexpr auto nt = native_type::text;
  static constexpr auto is_null = false;
  static bool parse(const SQLite::Column& column, std::vector<T>& out) {
    std::string str;
    bool ok = parser_column<std::string>::parse(column, str);
    if (!ok) {
      return false;
    }
    out = from_json_str<std::vector<T>>(str);
    return true;
  }
};

template <typename T>
  requires magic_enum::is_scoped_enum_v<T>
struct parser_column<T> {
  static constexpr auto nt = native_type::text;
  static constexpr auto is_null = false;
  static bool parse(const SQLite::Column& column, T& out) {
    std::string str;
    bool ok = parser_column<std::string>::parse(column, str);
    if (!ok) {
      return false;
    }
    auto r = magic_enum::enum_cast<T>(str);
    if (!r) {
      TGBM_LOG_ERROR("Fail parse `{}` from `{}`", name_type_v<T>, str);
      return false;
    }
    out = *r;
    return true;
  }
};

template <typename T>
struct parser_column<tgbm::api::optional<T>> {
  static constexpr auto nt = parser_column<T>::nt;
  static constexpr auto is_null = true;
  static bool parse(const SQLite::Column& column, tgbm::api::optional<T>& out) {
    if (column.isNull()) {
      out = std::nullopt;
      return true;
    }
    bool ok = parser_column<T>::parse(column, out.emplace());
    if (!ok) {
      TGBM_LOG_ERROR("Fail parse `{}`", name_type_v<T>);
      return false;
    }
    return true;
  }
};

}  // namespace bot::sql

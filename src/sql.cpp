#include "sql_utils.hpp"
#include "sqlite3.h"
#include "tgbm_replace/logger.hpp"

namespace bot::sql {

native_type from_sqlite_int(int type) {
  switch (type) {
    case SQLITE_INTEGER:
      return native_type::integer;
    case SQLITE_FLOAT:
      return native_type::floating;
    case SQLITE_BLOB:
      return native_type::blob;
    case SQLITE_TEXT:
      return native_type::text;
    case SQLITE_NULL:
      return native_type::null;
  }
  tgbm::unreachable();
}

bool parse_native_type(std::string_view str, native_type& out) {
  if (str == "INTEGER")
    out = native_type::integer;
  else if (str == "TEXT")
    out = native_type::text;
  else if (str == "BLOB")
    out = native_type::blob;
  else if (str == "NULL")
    out = native_type::null;
  else if (str == "FLOAT")
    out = native_type::floating;
  else
    return false;
  return true;
}

bool parser_column<bool>::parse(const SQLite::Column& col, bool& out) {
  if (!col.isInteger()) {
    return false;
  }
  out = col.getInt64() == 1;
  return true;
}

bool parser_column<int64_t>::parse(const SQLite::Column& col, int64_t& out) {
  if (!col.isInteger()) {
    return false;
  }
  out = col.getInt64();
  return true;
}

bool parser_column<json_value>::parse(const SQLite::Column& col, json_value& out) {
  if (!col.isText()) {
    return false;
  }
  out = json_value{boost::json::parse(col.getString())};
  return true;
}

bool parser_column<std::string>::parse(const SQLite::Column& col, std::string& out) {
  if (!col.isText()) {
    return false;
  }
  out = col.getString();
  return true;
}

bool parser_column<native_type>::parse(const SQLite::Column& col, native_type& out) {
  if (!col.isText()) {
    return false;
  }
  return parse_native_type(col.getString(), out);
}

}  // namespace bot::sql

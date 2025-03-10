#include "sql_utils.hpp"
#include "sqlite3.h"
#include "tgbm_replace/logger.hpp"

namespace bot::sql {

std::string_view type_column(const SQLite::Column& column) {
  const int INTEGER = SQLITE_INTEGER;
  const int FLOAT = SQLITE_FLOAT;
  const int TEXT = SQLITE_TEXT;
  const int BLOB = SQLITE_BLOB;
  const int Null = SQLITE_NULL;
  switch (column.getType()) {
    case SQLITE_INTEGER:
      return "integer";
    case SQLITE_FLOAT:
      return "float";
    case SQLITE_BLOB:
      return "blot";
    case SQLITE_NULL:
      return "null";
    case SQLITE_TEXT:
      return "text";
  }
  return "unknown";
}

#define LOG_ERROR_TYPE(EXPECTED_TYPE)                                                                   \
  TGBM_LOG_CRIT(                                                                                        \
      "Error while executing query: [{}]. Missmatch types on column with index {} and name `{}`, got: " \
      "[{}], "                                                                                          \
      "expected: "                                                                                      \
      "[{}]",                                                                                           \
      statement.getQuery(), index, col.getName(), type_column(col), #EXPECTED_TYPE);

bool parser_column<bool>::parse(SQLite::Statement& statement, std::size_t index) {
  if (statement.getColumnCount() <= index) {
    throw std::runtime_error("too big index");
  }
  auto col = statement.getColumn(index);
  if (!col.isInteger()) {
    LOG_ERROR_TYPE(Integer);
    throw std::runtime_error("Unexpected type");
  }
  return col.getInt64() == 1;
}

int64_t parser_column<int64_t>::parse(SQLite::Statement& statement, std::size_t index) {
  if (statement.getColumnCount() <= index) {
    throw std::runtime_error("too big index");
  }
  auto col = statement.getColumn(index);
  if (!col.isInteger()) {
    LOG_ERROR_TYPE(Integer);
    throw std::runtime_error("Unexpected type");
  }
  return col.getInt64();
}

json_value parser_column<json_value>::parse(SQLite::Statement& statement, std::size_t index) {
  if (statement.getColumnCount() <= index) {
    throw std::runtime_error("too big index");
  }
  auto col = statement.getColumn(index);
  if (!col.isText()) {
    LOG_ERROR_TYPE(Text);
    throw std::runtime_error("Unexpected type");
  }
  auto res = boost::json::parse(col.getString());
  if (!res.is_object()) {
    throw std::runtime_error("Unexpected type");
  }
  return json_value(std::move(res.as_object()));
}

std::string parser_column<std::string>::parse(SQLite::Statement& statement, std::size_t index) {
  if (statement.getColumnCount() <= index) {
    throw std::runtime_error("too big index");
  }
  auto col = statement.getColumn(index);
  if (!col.isText()) {
    LOG_ERROR_TYPE(Text);
    throw std::runtime_error("Unexpected type");
  }
  return col.getString();
}

ts_t parser_column<ts_t>::parse(SQLite::Statement& statement, std::size_t index) {
  if (statement.getColumnCount() <= index) {
    throw std::runtime_error("too big index");
  }
  auto col = statement.getColumn(index);
  if (!col.isText()) {
    LOG_ERROR_TYPE(Text);
    throw std::runtime_error("Unexpected type");
  }
  return parse_ts(col.getString());
}

io_event_meta parser_column<io_event_meta>::parse(SQLite::Statement& statement, std::size_t index) {
  if (statement.getColumnCount() <= index) {
    throw std::runtime_error("too big index");
  }
  auto col = statement.getColumn(index);
  if (!col.isText()) {
    LOG_ERROR_TYPE(Text);
    throw std::runtime_error("Unexpected type");
  }
  return from_json_str<io_event_meta>(col.getString());
}

time_event_meta parser_column<time_event_meta>::parse(SQLite::Statement& statement, std::size_t index) {
  if (statement.getColumnCount() <= index) {
    throw std::runtime_error("too big index");
  }
  auto col = statement.getColumn(index);
  if (!col.isText()) {
    LOG_ERROR_TYPE(Text);
    throw std::runtime_error("Unexpected type");
  }
  return from_json_str<time_event_meta>(col.getString());
}

}  // namespace bot::sql

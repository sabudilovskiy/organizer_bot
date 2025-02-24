#include "sql_utils.hpp"

namespace bot::sql {

bool parser_column<bool>::parse(SQLite::Statement& statement, std::size_t index) {
  if (statement.getColumnCount() <= index) {
    throw std::runtime_error("too big index");
  }
  auto col = statement.getColumn(index);
  if (!col.isInteger()) {
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
    throw std::runtime_error("Unexpected type");
  }
  return parse_ts(col.getString());
}
}  // namespace bot::sql

#pragma once

#include <boost/json.hpp>

#include <fmt/format.h>
#include <tgbm/api/optional.hpp>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Database.h>

#include "sql/binder.hpp"
#include "sql/parser.hpp"

namespace bot::sql {

template <typename Res, typename... Args>
Res execute(std::unique_ptr<SQLite::Database>& db, const std::string& query, const Args&... args) {
  SQLite::Statement statement(*db, query);
  std::size_t cur_index = 1;
  (binder<Args>::bind(statement, args, cur_index), ...);
  return parser<Res>::parse(statement);
}

// use fmt::format instead of bind
// only for queries like pragma
template <typename Res, typename... Args>
Res execute_unsafe(std::unique_ptr<SQLite::Database>& db, fmt::format_string<Args...> str, Args&&... args) {
  SQLite::Statement statement(*db, fmt::format(str, std::forward<Args>(args)...));
  return parser<Res>::parse(statement);
}

// does not add a comma at the beginning and end query
inline void add_n_arg(std::string& query, std::size_t N) {
  query.reserve(query.size() + N * 2 + 1);
  if (N > 0)
    query.append("?");
  for (std::size_t i = 1; i < N; i++) {
    query.append(",?");
  }
}

}  // namespace bot::sql

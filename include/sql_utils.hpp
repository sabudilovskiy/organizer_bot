#pragma once

#include <boost/json.hpp>

#include <fmt/format.h>
#include <tgbm/api/optional.hpp>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Database.h>
#include <ranges>

#include "sql/binder.hpp"
#include "sql/parser.hpp"

namespace bot::sql {

template <typename Res, typename... Args>
Res execute(std::unique_ptr<SQLite::Database>& db, const std::string& query,
            const Args&... args) {
  SQLite::Statement statement(*db, query);
  std::size_t cur_index = 1;
  (binder<Args>::bind(statement, args, cur_index), ...);
  return parser<Res>::parse(statement);
}

// use fmt::format instead of bind
// only for queries like pragma
template <typename Res, typename... Args>
Res execute_unsafe(std::unique_ptr<SQLite::Database>& db, fmt::format_string<Args...> str,
                   Args&&... args) {
  SQLite::Statement statement(*db, fmt::format(str, std::forward<Args>(args)...));
  return parser<Res>::parse(statement);
}

inline auto n_placeholders(std::size_t N) {
  return fmt::format("{}", std::views::repeat("?", N) | views::join(","));
}

}  // namespace bot::sql

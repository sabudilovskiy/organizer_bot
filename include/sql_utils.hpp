#pragma once

#include <boost/json.hpp>

#include <cstdint>
#include <stdexcept>
#include <tgbm/api/optional.hpp>
#include <type_traits>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Database.h>

#include "json.hpp"
#include "sql/binder.hpp"
#include "sql/parser.hpp"
#include "time.hpp"

namespace bot::sql {

template <typename Res, typename... Args>
Res execute(std::unique_ptr<SQLite::Database>& db, const std::string& query, const Args&... args) {
  SQLite::Statement statement(*db, query);
  std::size_t cur_index = 1;
  (binder<Args>::bind(statement, args, cur_index), ...);
  return parser<Res>::parse(statement);
}

}  // namespace bot::sql

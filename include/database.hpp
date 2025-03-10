#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

#include "sql_utils.hpp"
#include "time.hpp"

namespace bot {

inline const std::string q_create_migrations_table =
    "CREATE TABLE IF NOT EXISTS migrations ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "applied_at TEXT NOT NULL,"
    "migration_sql TEXT NOT NULL"
    ");";

inline const std::string q_get_migration_count = "SELECT id, applied_at, migration_sql FROM migrations";

inline const std::string q_insert_migration =
    "INSERT INTO migrations (applied_at, migration_sql) VALUES (?, ?)";

constexpr char q_get_table_info[] = "PRAGMA table_info({});";

struct PragmaInfo {
  std::int64_t cid;
  std::string name;
  sql::native_type type;
  bool notnull;
  tgbm::api::optional<std::string> dflt_value;
  bool pk;
};

struct Migration {
  std::int64_t id;
  ts_t applied_at;
  std::string migration_sql;
};

template <typename... Tables>
struct Database {
  using database = Database<Tables...>;

  Database(const std::string& dbPath, const auto& migrations);

  std::vector<PragmaInfo> get_info(const std::string& table) {
    return sql::execute_unsafe<std::vector<PragmaInfo>>(db, q_get_table_info, table);
  }

  std::vector<Migration> get_migrations() {
    return sql::execute<std::vector<Migration>>(db, q_get_migration_count);
  }

  template <typename T, typename... Args>
  T execute(const std::string& query, const Args&... args) {
    return sql::execute<T>(db, query, args...);
  }

  template <typename Res, typename... Args>
  Res execute_unsafe(fmt::format_string<Args...> str, Args&&... args) {
    return sql::execute_unsafe<Res>(db, str, std::forward<Args>(args)...);
  }

  void check_all_tables() {
    bool failed = false;
    (check_table<Tables>(failed), ...);
    if (failed)
      throw std::runtime_error("missmatch types in database");
  }

  template <typename T>
  void check_table(bool& failed);

 protected:
  std::unique_ptr<SQLite::Database> db;
};

template <typename... Tables>
Database<Tables...>::Database(const std::string& dbPath, const auto& migrations) {
  db = std::make_unique<SQLite::Database>(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  sql::execute<void>(db, q_create_migrations_table);
  // Get the number of applied migrations
  auto applied_migrations = get_migrations();
  if (applied_migrations.size() > migrations.size()) {
    TGBM_LOG_CRIT(
        "Detected incompatible version of database. Application expects a version equal or less {:03}, but "
        "dabase has {:03}",
        migrations.size(), applied_migrations.size());
    throw std::runtime_error("Incompatible version of database");
  }
  std::size_t idx = 0;
  for (auto& migration : migrations) {
    if (idx >= applied_migrations.size()) {
      try {
        db->exec(migration);
        sql::execute<void>(db, q_insert_migration, now(), migration);

        TGBM_LOG_INFO("Migration {:03} successfully applied", idx);
      } catch (const std::exception& exc) {
        TGBM_LOG_CRIT("Error applying migration {:03}: {}", idx, exc.what());
        throw std::runtime_error("Failed init database");
      }
    } else if (auto& applied_migration = applied_migrations[idx];
               migration == applied_migration.migration_sql) {
      TGBM_LOG_INFO("Migration {:03} is already applied, skipping", idx);
    } else {
      TGBM_LOG_CRIT(
          "Detected migration modification for {:03}.sql! "
          "Applied migration differs from stored version.",
          idx);
      throw std::runtime_error("Migration integrity violation detected");
    }

    idx++;
  }
  check_all_tables();
}

template <typename... Tables>
template <typename T>
void Database<Tables...>::check_table(bool& failed) {
  std::string db_name(T::db_name);
  auto db_columns = get_info(db_name);
  std::vector<std::string_view> exp_names = []() {
    auto names = boost::pfr::names_as_array<T>();
    std::vector<std::string_view> out(names.begin(), names.end());
    return out;
  }();
  std::vector<std::string_view> got_names = [&]() {
    std::vector<std::string_view> out;
    for (auto& col : db_columns) {
      out.emplace_back(col.name);
    }
    return out;
  }();

  if (exp_names != got_names) {
    TGBM_LOG_CRIT("Table {} has incorrect columns. Expected: [{}], got: [{}]", db_name,
                  fmt::join(exp_names, ","), fmt::join(got_names, ","));
    failed = true;
    return;
  }

  auto str_notnull = [](bool b) { return b ? "notnull" : "nullable"; };

  pfr_extension::visit_struct<T>([&]<typename Info, typename F>() {
    std::string name = std::string(Info::name.AsStringView());
    std::size_t index = Info::index;
    const auto& db_col = db_columns[index];
    sql::native_type exp_type = sql::parser_column<F>::nt;
    bool exp_notnull = !sql::parser_column<F>::is_null;
    // clang-format off
  if (
    db_col.type != exp_type || 
    db_col.notnull != exp_notnull
  )
  {
    TGBM_LOG_CRIT(
        "Mismatch in table {} in {} column '{}'. "
        "Expected ({}, {}) "
        "Got({}, {})",
        db_name, index, name, 
        exp_type, str_notnull(exp_notnull), 
        db_col.type, str_notnull(db_col.notnull));
    failed = true;
  }
    // clang-format on

    ++index;
  });
}

}  // namespace bot

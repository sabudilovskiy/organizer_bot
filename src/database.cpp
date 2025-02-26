#include "database.hpp"

#include <stdexcept>
#include <ranges>

#include "formatters/all.hpp"
#include "migrations/migrations_all.hpp"
#include "sql_utils.hpp"
#include "tgbm_replace/logger.hpp"
#include "types.hpp"

namespace bot {

const std::string q_insert_tasks =
    "INSERT INTO tasks (user_id, title, description, status, created_at) VALUES (?, "
    "?, ?, 0, datetime('now'))";
const std::string q_delete_task = "DELETE FROM tasks WHERE id = ? AND user_id = ?";
const std::string q_update_task =
    "UPDATE tasks SET title = ?, description = ?, status = ? WHERE id = ? AND user_id = ?";
const std::string q_get_all_tasks =
    "SELECT id, user_id, title, description, status, created_at FROM tasks WHERE user_id = ?";
const std::string q_filter_by_status =
    "SELECT id, user_id, title, description, status, created_at FROM tasks WHERE "
    "user_id = ? AND status = ?";
const std::string q_search_tasks_by_keyword =
    "SELECT id, user_id, title, description, status, created_at "
    "FROM tasks WHERE user_id = ? AND (title LIKE ? OR description LIKE ?)";
const std::string q_find_task_by_id =
    "SELECT id, user_id, title, description, status, created_at "
    "FROM tasks WHERE user_id = ? AND id = ?";

const std::string q_insert_user =
    "INSERT INTO users (user_id, chat_id, message_id, additional_messages, gmt_offset_m) VALUES (?, ?, ?, ?, "
    "?)";
const std::string q_get_user =
    "SELECT user_id, chat_id, message_id, additional_messages, gmt_offset_m FROM users WHERE user_id = ?";
const std::string q_update_user =
    "UPDATE users SET message_id = ?, additional_messages = ?, gmt_offset_m = ? WHERE user_id = ?";

const std::string q_get_events = "SELECT ts, user_id, meta FROM events";

const std::string q_add_event =
    "INSERT INTO events (ts, user_id, meta, consumed) VALUES (?, ?, ?, 0) RETURNING event_id";

const std::string q_select_events =
    "SELECT event_id, user_id, ts, consumed, meta FROM events where consumed = 0 ORDER BY ts ASC, "
    "user_id ASC";

const std::string q_create_migrations_table =
    "CREATE TABLE IF NOT EXISTS migrations ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "applied_at TEXT NOT NULL,"
    "migration_sql TEXT NOT NULL"
    ");";

const std::string q_get_migration_count = "SELECT id, applied_at, migration_sql FROM migrations";

const std::string q_insert_migration = "INSERT INTO migrations (applied_at, migration_sql) VALUES (?, ?)";

constexpr char q_get_table_info[] = "PRAGMA table_info({});";

void Database::addTask(std::int64_t user_id, const std::string& title, const std::string& description) {
  sql::execute<void>(db, q_insert_tasks, user_id, title, description);
}

void Database::deleteTask(std::int64_t id, std::int64_t user_id) {
  sql::execute<void>(db, q_delete_task, id, user_id);
}

void Database::updateTask(std::int64_t id, std::int64_t user_id, const std::string& title,
                          const std::string& description, bool status) {
  sql::execute<void>(db, q_update_task, id, user_id, title, description, status);
}

std::vector<Task> Database::getAllTasks(std::int64_t user_id) {
  return sql::execute<std::vector<Task>>(db, q_get_all_tasks, user_id);
}

std::vector<Task> Database::filterTasksByStatus(std::int64_t user_id, bool status) {
  return sql::execute<std::vector<Task>>(db, q_filter_by_status, user_id, status);
}

std::vector<Task> Database::searchTasks(std::int64_t user_id, const std::string& keyword) {
  std::string likeKeyword = "%" + keyword + "%";
  return sql::execute<std::vector<Task>>(db, q_search_tasks_by_keyword, user_id, likeKeyword, likeKeyword);
}

template <typename T>
void check_table(Database& db, bool& failed) {
  std::string db_name(T::db_name);
  auto db_columns = db.get_info(db_name);
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

template <typename... Ts>
void check_all_tables(Database& db, aa::type_list<Ts...>) {
  bool failed = false;
  (check_table<Ts>(db, failed), ...);
  if (failed)
    throw std::runtime_error("missmatch types in database");
}

void Database::start() {
  sql::execute<void>(db, q_create_migrations_table);
  // Get the number of applied migrations
  auto applied_migrations = get_migrations();
  if (applied_migrations.size() > sql::migrations.size()) {
    TGBM_LOG_CRIT(
        "Detected incompatible version of database. Application expects a version equal or less {:03}, but "
        "dabase has {:03}",
        sql::migrations.size(), applied_migrations.size());
    throw std::runtime_error("Incompatible version of database");
  }
  std::size_t idx = 0;
  for (auto& migration : sql::migrations) {
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
  check_all_tables(*this, Tables{});
}

Database::Database(const std::string& dbPath) {
  // Открытие или создание базы данных
  db = std::make_unique<SQLite::Database>(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  start();
}

User Database::fetchUser(const RequestUser& user) {
  if (auto fetched = sql::execute<tgbm::api::optional<User>>(db, q_get_user, user.user_id)) {
    return std::move(*fetched);
  } else if (user.chat_id) {
    User new_user(user.user_id, *user.chat_id);
    sql::execute<void>(db, q_insert_user, new_user);
    return new_user;
  } else {
    throw std::runtime_error("not found user");
  }
}

void Database::updateUser(const User& user) {
  sql::execute<void>(db, q_update_user, user.message_id, user.additional_messages, user.user_id,
                     user.gmt_offset_m);
}

tgbm::api::optional<Task> Database::find_task(std::int64_t user_id, std::int64_t task_id) {
  return sql::execute<tgbm::api::optional<Task>>(db, q_find_task_by_id, user_id, task_id);
}

std::vector<Event> Database::getEvents() {
  return sql::execute<std::vector<Event>>(db, q_select_events);
}

std::int64_t Database::addEvent(const Event& e) {
  return sql::execute<int64_t>(db, q_add_event, e.ts, e.user_id, e.meta);
}

void Database::consumeEvents(const std::vector<int64_t>& event_ids) {
  if (event_ids.empty()) {
    return;
  }
  std::string query = "UPDATE events SET consumed = true WHERE event_id IN (";
  sql::add_n_arg(query, event_ids.size());
  query.append(")");

  SQLite::Statement statement(*db, query);

  std::size_t cur_index = 1;
  for (auto event_id : event_ids) {
    sql::binder<int64_t>::bind(statement, event_id, cur_index);
  }
  while (statement.executeStep()) {
  }
}

std::vector<PragmaInfo> Database::get_info(const std::string& table) {
  return sql::execute_unsafe<std::vector<PragmaInfo>>(db, q_get_table_info, table);
}

std::vector<Migration> Database::get_migrations() {
  return sql::execute<std::vector<Migration>>(db, q_get_migration_count);
}
}  // namespace bot

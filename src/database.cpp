#include "database.hpp"

#include <stdexcept>

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
    "INSERT INTO users (user_id, chat_id, message_id, additional_messages) VALUES (?, ?, ?, ?)";
const std::string q_get_user =
    "SELECT user_id, chat_id, message_id, additional_messages FROM users WHERE user_id = ?";
const std::string q_update_user =
    "UPDATE users SET message_id = ?, additional_messages = ? WHERE user_id = ?";

const std::string q_get_events = "SELECT ts, user_id, meta FROM events";

const std::string q_add_event =
    "INSERT INTO events (ts, user_id, meta, consumed) VALUES (?, ?, ?, 0) RETURNING event_id";

const std::string q_select_events =
    "SELECT event_id, user_id, ts, consumed, meta FROM events where consumed = 0 ORDER BY ts ASC, "
    "user_id ASC";

const std::string q_init = R"(

CREATE TABLE IF NOT EXISTS users (
      user_id INTEGER PRIMARY KEY, 
      chat_id INTEGER, 
      message_id INTEGER, 
      additional_messages INTEGER);

 CREATE TABLE IF NOT EXISTS tasks (
      id INTEGER PRIMARY KEY AUTOINCREMENT, 
      user_id INTEGER, 
      title TEXT, 
      description TEXT, 
      status INTEGER, 
      created_at TEXT);

CREATE TABLE IF NOT EXISTS events (
      event_id INTEGER PRIMARY KEY AUTOINCREMENT, 
      user_id INTEGER,
      ts TEXT, 
      meta TEXT, 
      consumed INTEGER
);

CREATE INDEX IF NOT EXISTS events_idx ON events (user_id, consumed, ts);
CREATE INDEX IF NOT EXISTS tasks_idx ON events (user_id);
)";

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

void Database::createTables() {
  try {
    db->exec(q_init);
  } catch (std::exception& exc) {
    TGBM_LOG_CRIT("Error while create tables: {}", exc.what());
  }
}

Database::Database(const std::string& dbPath) {
  // Открытие или создание базы данных
  db = std::make_unique<SQLite::Database>(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  createTables();
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
  sql::execute<void>(db, q_update_user, user.message_id, user.additional_messages, user.user_id);
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

}  // namespace bot

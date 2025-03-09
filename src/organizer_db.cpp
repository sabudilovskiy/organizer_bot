#include "organizer_db.hpp"

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

const std::string q_add_io_event =
    "INSERT INTO io_events (ts, user_id, meta, consumed) VALUES (?, ?, ?, 0) RETURNING io_event_id";

const std::string q_select_io_events =
    "SELECT io_event_id, user_id, ts, meta, consumed FROM io_events where consumed = 0 ORDER BY ts ASC, "
    "user_id ASC";

constexpr char q_consume_io_events[] = "UPDATE io_events SET consumed = true WHERE io_event_id IN ({})";

const std::string q_insert_call =
    "INSERT INTO calls (user_id, name, description, schedule) VALUES (?,?,?,?) RETURNING call_id";

const std::string q_get_calls =
    "SELECT call_id, user_id, name, description, schedule FROM calls WHERE user_id = ?";

void OrganizerDB::addTask(std::int64_t user_id, const std::string& title, const std::string& description) {
  execute<void>(q_insert_tasks, user_id, title, description);
}

void OrganizerDB::deleteTask(std::int64_t id, std::int64_t user_id) {
  execute<void>(q_delete_task, id, user_id);
}

void OrganizerDB::updateTask(std::int64_t id, std::int64_t user_id, const std::string& title,
                             const std::string& description, bool status) {
  execute<void>(q_update_task, id, user_id, title, description, status);
}

std::vector<Task> OrganizerDB::getAllTasks(std::int64_t user_id) {
  return execute<std::vector<Task>>(q_get_all_tasks, user_id);
}

std::vector<Task> OrganizerDB::filterTasksByStatus(std::int64_t user_id, bool status) {
  return execute<std::vector<Task>>(q_filter_by_status, user_id, status);
}

std::vector<Task> OrganizerDB::searchTasks(std::int64_t user_id, const std::string& keyword) {
  std::string likeKeyword = "%" + keyword + "%";
  return execute<std::vector<Task>>(q_search_tasks_by_keyword, user_id, likeKeyword, likeKeyword);
}

OrganizerDB::OrganizerDB(const std::string& dbPath) : database(dbPath, sql::migrations) {
}

User OrganizerDB::fetchUser(const RequestUser& user) {
  if (auto fetched = execute<tgbm::api::optional<User>>(q_get_user, user.user_id)) {
    return std::move(*fetched);
  } else if (user.chat_id) {
    User new_user(user.user_id, *user.chat_id);
    execute<void>(q_insert_user, sql::as_sequence(new_user));
    return new_user;
  } else {
    throw std::runtime_error("not found user");
  }
}

void OrganizerDB::updateUser(const User& user) {
  execute<void>(q_update_user, user.message_id, user.additional_messages, user.user_id, user.gmt_offset_m);
}

tgbm::api::optional<Task> OrganizerDB::find_task(std::int64_t user_id, std::int64_t task_id) {
  return execute<tgbm::api::optional<Task>>(q_find_task_by_id, user_id, task_id);
}

std::vector<io_event> OrganizerDB::getEvents() {
  return execute<std::vector<io_event>>(q_select_io_events);
}

std::int64_t OrganizerDB::addEvent(const io_event& e) {
  return execute<int64_t>(q_add_io_event, e.ts, e.user_id, e.meta);
}

void OrganizerDB::consumeEvents(const std::vector<int64_t>& event_ids) {
  if (event_ids.empty()) {
    return;
  }
  std::string query = fmt::format(q_consume_io_events, sql::n_placeholders(event_ids.size()));
  execute<void>(query, sql::as_sequence(event_ids));
}

std::int64_t OrganizerDB::addCall(const Call& call) {
  return execute<std::int64_t>(q_insert_call, call.user_id, call.name, call.description, call.schedule);
}

std::vector<Call> OrganizerDB::getCalls(std::int64_t user_id) {
  return execute<std::vector<Call>>(q_get_calls, user_id);
}
}  // namespace bot

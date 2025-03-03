#pragma once

#include <anyany/noexport/common.hpp>
#include <vector>
#include <tgbm/api/optional.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

#include "event.hpp"
#include "sql/native_type.hpp"
#include "types.hpp"

namespace bot {

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

struct Database {
  using Tables = aa::type_list<Event, Task, User>;

  Database(const std::string& dbPath);

  User fetchUser(const RequestUser& user);

  void updateUser(const User& user);

  void addTask(std::int64_t user_id, const std::string& title, const std::string& description);

  void deleteTask(std::int64_t id, std::int64_t user_id);

  void updateTask(std::int64_t id, std::int64_t user_id, const std::string& title,
                  const std::string& description, bool status);

  void consumeEvents(const std::vector<int64_t>& event_ids);

  std::vector<PragmaInfo> get_info(const std::string& table);

  std::vector<Migration> get_migrations();

  // returns: event_id
  // ignore event_id from
  std::int64_t addEvent(const Event& event);

  std::vector<Event> getEvents();

  tgbm::api::optional<Task> find_task(std::int64_t user_id, std::int64_t task_id);

  std::vector<Task> getAllTasks(std::int64_t user_id);

  std::vector<Task> filterTasksByStatus(std::int64_t user_id, bool status);

  std::vector<Task> searchTasks(std::int64_t user_id, const std::string& keyword);

  // returns: call_id
  // ignore call_id from
  std::int64_t addCall(const Call& call);

  std::vector<Call> getCalls(std::int64_t user_id);

 private:
  std::unique_ptr<SQLite::Database> db;

  void start();
};

}  // namespace bot

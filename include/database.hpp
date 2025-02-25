#pragma once

#include <vector>
#include <tgbm/api/optional.hpp>
#include <SQLiteCpp/SQLiteCpp.h>
#include <optional>

#include "event.hpp"
#include "types.hpp"

namespace bot {

struct Database {
  Database(const std::string& dbPath);

  User fetchUser(const RequestUser& user);

  void updateUser(const User& user);

  void addTask(std::int64_t user_id, const std::string& title, const std::string& description);

  void deleteTask(std::int64_t id, std::int64_t user_id);

  void updateTask(std::int64_t id, std::int64_t user_id, const std::string& title,
                  const std::string& description, bool status);

  void consumeEvents(const std::vector<int64_t>& event_ids);

  // returns: event_id
  // ignore event_id from
  std::int64_t addEvent(const Event& event);

  std::vector<Event> getEvents();

  tgbm::api::optional<Task> find_task(std::int64_t user_id, std::int64_t task_id);

  std::vector<Task> getAllTasks(std::int64_t user_id);

  std::vector<Task> filterTasksByStatus(std::int64_t user_id, bool status);

  std::vector<Task> searchTasks(std::int64_t user_id, const std::string& keyword);

 private:
  std::unique_ptr<SQLite::Database> db;

  void createTables();
};

}  // namespace bot

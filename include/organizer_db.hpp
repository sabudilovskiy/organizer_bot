#pragma once

#include <anyany/noexport/common.hpp>
#include <vector>
#include <tgbm/api/optional.hpp>
#include <SQLiteCpp/SQLiteCpp.h>
#include <unordered_set>

#include "database.hpp"
#include "io_event.hpp"
#include "time_event.hpp"
#include "types.hpp"

namespace bot {

struct OrganizerDB : Database<io_event, time_event, Task, User> {
  OrganizerDB(const std::string& dbPath);

  User fetchUser(const RequestUser& user);

  void updateUser(const User& user);

  void addTask(std::int64_t user_id, const std::string& title,
               const std::string& description);

  void deleteTask(std::int64_t id, std::int64_t user_id);

  void updateTask(std::int64_t id, std::int64_t user_id, const std::string& title,
                  const std::string& description, bool status);

  void consumeIoEvents(const std::vector<int64_t>& event_ids);

  // returns: event_id
  // ignore event_id from
  std::int64_t addEvent(const io_event& event);

  std::vector<io_event> getEvents();

  tgbm::api::optional<Task> find_task(std::int64_t user_id, std::int64_t task_id);

  std::vector<Task> getAllTasks(std::int64_t user_id);

  std::vector<Task> filterTasksByStatus(std::int64_t user_id, bool status);

  std::vector<Task> searchTasks(std::int64_t user_id, const std::string& keyword);

  // returns: call_id
  // ignore call_id from
  std::int64_t addCall(const Call& call);

  std::vector<Call> getCalls(std::int64_t user_id);

  std::int64_t addTimeEvent(const time_event& event);

  void consumeTimeEvents(const std::vector<int64_t>& event_ids);

  void consumeTimeEvents(const std::unordered_set<int64_t>& event_ids);

  std::vector<time_event> getTimeEvents(ts_utc_t max_time);

  std::vector<time_event> getUserTimeEventsByType(std::int64_t user_id,
                                                  time_event_type type);
};

}  // namespace bot

#pragma once

#include <string_view>

namespace bot {
namespace commands {
constexpr std::string_view k_add_task = "Add task";
constexpr std::string_view k_delete_task = "Add task";
constexpr std::string_view k_update_task = "Add task";
constexpr std::string_view k_list_tasks = "List all tasks";
constexpr std::string_view k_filter = "Filter by keyword";
}  // namespace commands
namespace states {
constexpr std::string_view k_init = "init";
constexpr std::string_view k_commands = "commands";

constexpr std::string_view k_add_task_init = "add_task_init";
constexpr std::string_view k_add_task_title = "add_task_title";
constexpr std::string_view k_add_task_description = "add_task_description";

constexpr std::string_view k_list_init = "list_tasks_init";

}  // namespace states
}  // namespace bot

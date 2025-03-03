#pragma once

#include <boost/json.hpp>
#include <consts.hpp>
#include <cstdint>
#include <string>
#include <tgbm/api/optional.hpp>

#include "json/value.hpp"
#include "time.hpp"

namespace bot {

struct Task {
  std::int64_t id;
  std::int64_t user_id;
  std::string title;
  std::string description;
  bool status;
  std::string created_at;

  static constexpr std::string_view db_name = "tasks";
};

struct RequestUser {
  int64_t user_id;
  tgbm::api::optional<int64_t> chat_id;
};

struct User {
  int64_t user_id;
  int64_t chat_id;
  tgbm::api::optional<int64_t> message_id;
  int64_t additional_messages = 0;
  int64_t gmt_offset_m = 90;

  bool need_new_message() {
    return additional_messages >= max_additional_messages;
  }

  void set_need_new_message() {
    additional_messages = max_additional_messages;
  }

  static constexpr std::string_view db_name = "users";
  static constexpr int64_t max_additional_messages = 1;
};

struct Call {
  std::int64_t call_id;
  std::int64_t user_id;
  std::string name;
  std::string description;
  schedule_unit schedule;

  static constexpr std::string_view db_name = "calls";
};

}  // namespace bot

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

  static constexpr int64_t max_additional_messages = 4;

  bool need_new_message() {
    return additional_messages > max_additional_messages;
  }

  void set_need_new_message() {
    additional_messages = max_additional_messages;
  }
};

}  // namespace bot

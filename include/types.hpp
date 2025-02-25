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
  std::string state = std::string(states::k_init);
  json_value meta = json_value::object();

  void reset() noexcept {
    state = std::string(states::k_init);
    meta = json_value::object();
  }
};

}  // namespace bot

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

struct UserSettings {
  std::vector<time_of_day> call_everyday_notifies;
  int64_t gmt_offset_m = 90;
};

struct User {
  int64_t user_id;
  int64_t chat_id;
  tgbm::api::optional<int64_t> message_id;
  int64_t additional_messages = 0;
  UserSettings settings;

  bool need_new_message();

  void set_need_new_message();

  static constexpr std::string_view db_name = "users";
  static constexpr int64_t max_additional_messages = 1;

  ts_t convert_to_utc(ts_t time) const;
  void convert_inplace_to_utc(ts_t& time) const;

  ts_t convert_to_user_time(ts_t time) const;
  void convert_inplace_to_user_time(ts_t& time) const;

  ts_t now() const;
};

struct Call {
  std::int64_t call_id;
  std::int64_t user_id;
  std::string name;
  std::string description;
  std::int64_t duration;
  schedule_unit schedule;

  time_of_day begin() const noexcept;

  time_of_day end() const noexcept;

  static constexpr std::string_view db_name = "calls";
};

}  // namespace bot

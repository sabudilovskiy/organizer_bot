#pragma once

#include <chrono>
namespace bot {

using ts_t = std::chrono::local_time<std::chrono::nanoseconds>;

ts_t now();
ts_t to_zoned_time(const ts_t& ts);

std::string to_string(const ts_t& ts);
ts_t parse_ts(const std::string& str);

}  // namespace bot

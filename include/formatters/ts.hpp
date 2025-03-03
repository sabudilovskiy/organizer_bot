#pragma once

#include <fmt/format.h>

#include "time.hpp"

namespace fmt {
template <>
struct formatter<::bot::ts_t> : fmt::formatter<std::string> {
  auto format(const bot::ts_t& t, auto& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", ::bot::to_string(t));
  }
};

template <>
struct formatter<::bot::weekday> : fmt::formatter<std::string> {
  auto format(const ::bot::weekday& t, auto& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", ::bot::to_string(t));
  }
};

template <>
struct formatter<::bot::time_of_day> : fmt::formatter<std::string> {
  auto format(const ::bot::time_of_day& t, auto& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{:02}:{:02}", t.hour, t.minute);
  }
};

}  // namespace fmt

#pragma once

#include <fmt/format.h>

#include "json/value.hpp"

namespace fmt {
template <>
struct formatter<bot::json_view> : fmt::formatter<std::string> {
  auto format(const bot::json_view& t, auto& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", t.serialize());
  }
};

template <>
struct formatter<bot::json_value> : fmt::formatter<std::string> {
  auto format(const bot::json_value& t, auto& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", t.serialize());
  }
};
}  // namespace fmt

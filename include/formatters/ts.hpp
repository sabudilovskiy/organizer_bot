#pragma once

#include <fmt/format.h>

#include "time.hpp"

namespace fmt {
template <>
struct formatter<bot::ts_t> : fmt::formatter<std::string> {
  auto format(const bot::ts_t& t, auto& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", ::bot::to_string(t));
  }
};
}  // namespace fmt

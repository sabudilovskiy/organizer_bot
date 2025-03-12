#pragma once

#include <tgbm/utils/traits.hpp>

#include "json/value.hpp"

namespace fmt {

template <typename T>
  requires tgbm::aggregate<T>
struct formatter<T> : fmt::formatter<std::string> {
  auto format(const T& t, auto& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", bot::to_json_str(t));
  }
};
}  // namespace fmt

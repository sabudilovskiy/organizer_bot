#pragma once

#include <fmt/format.h>

#include "time/time_of_day.hpp"
#include "traits.hpp"

namespace fmt {

template <bot::serializable T>
struct formatter<T> : fmt::formatter<std::string> {
  auto format(const T& t, auto& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", t.serialize());
  }
};

}  // namespace fmt

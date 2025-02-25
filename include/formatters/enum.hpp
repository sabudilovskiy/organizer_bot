#pragma once

#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>

namespace fmt {
template <typename T>
  requires magic_enum::is_scoped_enum_v<T>
struct formatter<T> : formatter<std::string> {
  auto format(const T& t, auto& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", magic_enum::enum_name(t));
  }
};
}  // namespace fmt

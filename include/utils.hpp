#pragma once

#include <fmt/format.h>

namespace bot {

template <typename T = std::runtime_error, typename... Args>
[[noreturn]] void throw_formatted(fmt::format_string<Args...> fmt_str, Args&&... args) {
  throw T(fmt::format(fmt_str, std::forward<Args>(args)...));
}

}  // namespace bot

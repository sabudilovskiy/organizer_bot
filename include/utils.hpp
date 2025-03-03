#pragma once

#include <fmt/format.h>

namespace bot {

template <typename T>
constexpr std::string_view name_of_type() {
  std::string_view res = "";
#if defined(__GNUC__) && !defined(__clang__)
  res = __PRETTY_FUNCTION__;
  res.remove_prefix(sizeof("consteval std::string_view bot::name_of_type() [with T ="));
  auto idx = res.find(";");
  res.remove_suffix(res.size() - idx);
#elif defined(__clang__)
  res = __PRETTY_FUNCTION__;
  res.remove_prefix(sizeof("std::string_view bot::name_of_type() [T ="));
  res.remove_suffix(1);
#elif defined(_MSC_VER)
  res = __FUNCSIG__;
  //   res.remove_prefix(sizeof(""));
  // MSVC may return different names for SAME type if it was decalred as struct and implemented as class or
  // smth like
  auto start = res.find("name_of_type");
  res.remove_prefix(start + sizeof("name_of_type"));
  res.remove_suffix(sizeof(">(void)") - 1);
  if (res.starts_with("class"))
    res.remove_prefix(sizeof("class"));
  else if (res.starts_with("struct"))
    res.remove_prefix(sizeof("struct"));
  else if (res.starts_with("union"))
    res.remove_prefix(sizeof("union"));
  else if (res.starts_with("enum"))
    res.remove_prefix(sizeof("enum"));
    // fundamental types has no 'prefix'
#else
  #error Unknown compiler
#endif
  return res;
}

template <typename T>
inline constexpr std::string_view name_type = name_of_type<T>();

template <typename T = std::runtime_error, typename... Args>
[[noreturn]] void throw_formatted(fmt::format_string<Args...> fmt_str, Args&&... args) {
  throw T(fmt::format(fmt_str, std::forward<Args>(args)...));
}

}  // namespace bot

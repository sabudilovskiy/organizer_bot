#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ranges>
#include <variant>

#include <magic_enum/magic_enum.hpp>
#include <tgbm/utils/pfr_extension.hpp>
#include <tgbm/utils/meta.hpp>

namespace bot {

namespace details {
template <typename T>
constexpr std::string_view name_of_type() {
  std::string_view res = "";
#if defined(__GNUC__) && !defined(__clang__)
  res = __PRETTY_FUNCTION__;
  res.remove_prefix(
      sizeof("consteval std::string_view bot::details::name_of_type() [with T ="));
  auto idx = res.find(";");
  res.remove_suffix(res.size() - idx);
#elif defined(__clang__)
  res = __PRETTY_FUNCTION__;
  res.remove_prefix(sizeof("std::string_view bot::details::name_of_type() [T ="));
  res.remove_suffix(1);
#elif defined(_MSC_VER)
  res = __FUNCSIG__;
  //   res.remove_prefix(sizeof(""));
  // MSVC may return different names for SAME type if it was decalred as struct and
  // implemented as class or smth like
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
}  // namespace details

template <typename T>
struct name_type {
  static constexpr std::string_view value = details::name_of_type<T>();
};

template <typename T>
constexpr std::string_view name_type_v = name_type<T>::value;

inline constexpr std::string_view name_without_ns(std::string_view name) {
  auto found = name.find_last_of(":");
  if (found != name.npos) {
    name.remove_prefix(found + 1);
  }
  return name;
}

template <typename T = std::runtime_error, typename... Args>
[[noreturn]] void throw_formatted(fmt::format_string<Args...> fmt_str, Args&&... args) {
  throw T(fmt::format(fmt_str, std::forward<Args>(args)...));
}

template <typename Variant, typename T>
struct variant_index {
  static constexpr std::size_t value = []() {
    static constexpr std::size_t N = std::variant_size_v<Variant>;
    return []<std::size_t... I>(std::index_sequence<I...>) {
      std::array<bool, N> eq{
          std::is_same_v<std::variant_alternative_t<I, Variant>, T>...};
      return std::distance(eq.begin(), std::find(eq.begin(), eq.end(), 1));
    }(std::make_index_sequence<N>{});
  }();
};

template <typename Variant, typename T>
constexpr std::size_t variant_index_v = variant_index<Variant, T>::value;

template <typename Variant>
struct variant_names {
  static constexpr auto N = std::variant_size_v<Variant>;
  using value_type = std::array<std::string_view, N>;

  static constexpr value_type value = []<std::size_t... I>(std::index_sequence<I...>) {
    return value_type{
        name_without_ns(name_type_v<std::variant_alternative_t<I, Variant>>)...};
  }(std::make_index_sequence<N>{});
};

template <typename Variant>
constexpr std::array<std::string_view, variant_names<Variant>::N> variant_names_v =
    variant_names<Variant>::value;

namespace views {
struct join_adapter {
  std::string_view separator;

  template <typename Range>
  friend auto operator|(Range&& range, join_adapter adapter) {
    return fmt::join(std::forward<Range>(range), adapter.separator);
  }
};

template <typename Range>
auto join(Range&& range, std::string_view separator) {
  return join_adapter(separator) | std::forward<Range>(range);
}

inline auto join(std::string_view separator) {
  return join_adapter(separator);
}

}  // namespace views

inline std::string_view exc_what_coro(std::exception& exc) {
#ifndef _WIN32
  return exc.what();
#else
  return "info about exception unavailable on windows";
#endif
}

template <tgbm::ce::string Name, typename T>
constexpr decltype(auto) get_by_name(T&& t) {
  constexpr auto I = pfr_extension::tuple_element_index_v<Name, T>;
  static_assert(I < boost::pfr::tuple_size_v<T>, "name don't provided in tuple");
  return boost::pfr::get<I>(std::forward<T>(t));
}

template <typename Type, std::size_t... sizes>
constexpr auto array_cat(const std::array<Type, sizes>&... arrays) {
  std::array<Type, (sizes + ...)> result;
  std::size_t index{};

  ((std::copy_n(arrays.begin(), sizes, result.begin() + index), index += sizes), ...);

  return result;
}

template <typename T>
constexpr std::array<std::string_view, pfr_extension::tuple_size_v<T>> names_as_array_v =
    []() {
      if constexpr (boost::pfr::tuple_size_v<T> != 0) {
        return boost::pfr::names_as_array<std::remove_cvref_t<T>>();
      } else
        return std::array<std::string_view, 0>{};
    }();

template <typename... T>
struct set_of_names {
  static constexpr auto value = []() {
    constexpr auto result_with_nulls_with_size = []() {
      auto all_names = array_cat(names_as_array_v<std::remove_cvref_t<T>>...);
      std::sort(all_names.begin(), all_names.end());
      auto size = std::unique(all_names.begin(), all_names.end()) - all_names.begin();
      return std::make_pair(all_names, size);
    }();
    constexpr auto N = result_with_nulls_with_size.second;
    std::array<std::string_view, N> result{};
    std::copy_n(result_with_nulls_with_size.first.begin(), N, result.begin());
    return result;
  }();
};

template <typename... T>
constexpr auto set_of_names_v = set_of_names<T...>::value;

struct missing {};

namespace details {

template <tgbm::ce::string Name, typename U>
decltype(auto) get_if_exist(U& u) {
  constexpr std::size_t I = pfr_extension::tuple_element_index_v<Name, U>;
  if constexpr (boost::pfr::tuple_size_v<U> <= I) {
    return missing{};
  } else {
    return boost::pfr::get<I>(u);
  }
}

template <std::size_t I, typename... T>
void visit_one_field(auto& functor, T&... t) {
  constexpr auto set_of_names = set_of_names_v<T...>;
  constexpr tgbm::ce::string name(set_of_names[I]);
  functor.template operator()<name>(get_if_exist<name>(t)...);
}

}  // namespace details

template <typename... T>
void visit_all(auto&& functor, T&&... t) {
  constexpr auto set_of_names = set_of_names_v<T...>;
  constexpr std::size_t N = set_of_names.size();

  [&]<std::size_t... I>(std::index_sequence<I...>) {
    (details::visit_one_field<I>(functor, t...), ...);
  }(std::make_index_sequence<N>{});
}

}  // namespace bot

#pragma once

#include <concepts>
#include <string>

namespace bot {

template <typename T>
concept serializable = requires(const T& t) {
  { t.serialize() };
};

template <typename T>
concept parseable = requires(std::string_view str) {
  { T::parse(str) };
};

template <typename T>
concept parseable_inplace = requires(std::string_view str, T& out) {
  { T::parse(str, out) } -> std::same_as<bool>;
};

}  // namespace bot

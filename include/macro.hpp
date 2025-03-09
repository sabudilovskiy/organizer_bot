#pragma once

#include <kelcoro/generator.hpp>

namespace bot {

constexpr std::uint64_t constexpr_place_hash(std::string_view file) noexcept {
  std::uint64_t hash = 14695981039346656037ull;  // FNV-1a offset basis
  for (auto c : file) {
    hash ^= static_cast<std::uint8_t>(c);
    hash *= 1099511628211ull;  // FNV-1a prime
  }
  return hash;
}

}  // namespace bot

#define AWAIT_ALL(X) co_yield dd::elements_of(X)
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)
#define FILE_LINE __FILE__ ":" STRINGIFY(__LINE__)
#define ID() ::bot::constexpr_place_hash(FILE_LINE)

#ifndef _WIN32
  #define EXC_WHAT(EXC) exc.what()
#else
  #define EXC_WHAT(EXC) "info about exception unavailable on windows"
#endif

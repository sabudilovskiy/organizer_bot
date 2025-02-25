#pragma once
#include <stdexcept>

namespace bot {

struct invariant_error : std::runtime_error {
  using std::runtime_error::runtime_error;
};
}  // namespace bot

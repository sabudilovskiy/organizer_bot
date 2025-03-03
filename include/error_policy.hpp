#pragma once

namespace bot {

enum struct error_policy {
  use_optional,
  use_exceptions,
};

template <error_policy p>
struct error_policy_tag {};

using use_optional = error_policy_tag<error_policy::use_optional>;
using use_exceptions = error_policy_tag<error_policy::use_exceptions>;

}  // namespace bot

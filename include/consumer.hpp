#pragma once

#include <kelcoro/channel.hpp>
#include <kelcoro/generator.hpp>
#include <kelcoro/task.hpp>

namespace bot {

using consumer_t = dd::channel<dd::nothing_t>;
using action_t = dd::task<void>;

}  // namespace bot

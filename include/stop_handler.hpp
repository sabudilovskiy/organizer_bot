#pragma once

#include <atomic>

namespace bot {

void set_stop_handler();

void need_stop();

bool is_need_stop();

}  // namespace bot

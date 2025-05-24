#pragma once

#include "tgbm_replace/common.hpp"
#include "tgbm_replace/level.hpp"

// ============================================================================
// Log configuration interface.
//
// This file provides global runtime configuration for the logging system.
//
// Internally, all config is stored in relaxed atomics.
//     Changes are visible across threads eventually, but without synchronization
//     guarantees.
//
// Passing `nullptr` to any logging function setter is unsupported and
//     will result in a hard failure upon the first log call.
//     Use log level filtering instead.
//
// To disable logging entirely:
//     - Use the `TGBM_MIN_LOG_LEVEL` macro at compile time to strip logs
//     - Or call `set_log_level(...)` at runtime to filter by severity
//     - Or call `disable_logs()` to drop all logs immediately
//
// The logging system operates in a single execution mode:
//
//    Direct (unbuffered) logging:
//      - Each log is immediately formatted and passed to the `direct_sink_fn`
//      - Formatting is done before the sink call
//
// The following customization points are available:
//
//   - `set_direct_sink_fn()`:
//         Function to consume fully-formatted log lines
//
//   - `set_log_level(level)` / `get_log_level()`:
//         Sets or retrieves the minimum severity level for log messages.
//         Only messages with severity `>= get_log_level()` will be emitted.
//
// The timestamp type used is `system_clock::time_point` for wall-clock logging.
//
// Default configuration (if not overridden):
//   - `level`: debug
//   - `TGBM_MIN_LOG_LEVEL_VALUE`: debug
//   - `direct_sink_fn`: logs to per-thread file (`<tid>.thread_log`) with flush
// ============================================================================
namespace tgbm::log {

void set_direct_sink_fn(direct_sink_fn fn) noexcept;

void set_log_level(level lvl) noexcept;

direct_sink_fn get_direct_sink_fn() noexcept;
level get_log_level() noexcept;  // Returns the minimum level at which log messages are
                                 // emitted

}  // namespace tgbm::log

#ifndef TGBM_MIN_LOG_LEVEL
  #define TGBM_MIN_LOG_LEVEL ::tgbm::log::level::debug
#else
#endif

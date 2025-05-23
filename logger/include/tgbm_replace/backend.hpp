#pragma once

#include <string_view>

#include "tgbm_replace/common.hpp"
#include "tgbm_replace/level.hpp"

namespace tgbm::log {

namespace sink {

void std_out(level lvl, std::string_view str) noexcept;
void std_err(level lvl, std::string_view str) noexcept;
void std_out_colored(level lvl, std::string_view str) noexcept;
void std_err_colored(level lvl, std::string_view str) noexcept;

void per_thread_file(level lvl, std::string_view str) noexcept;
void unite_file(level lvl, std::string_view str) noexcept;

void per_thread_file_with_flush(level lvl, std::string_view str) noexcept;
void unite_file_with_flush(level lvl, std::string_view str) noexcept;
}  // namespace sink

namespace internal {

struct scoped_file;

scoped_file& get_thread_file();

scoped_file& get_unite_file();
}  // namespace internal

}  // namespace tgbm::log

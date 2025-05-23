#pragma once

#include <cstdio>
#include <string_view>

namespace tgbm::log::internal {

struct scoped_file {
  explicit scoped_file(FILE* file) noexcept;

  ~scoped_file();

  scoped_file(const scoped_file&) = delete;
  scoped_file& operator=(const scoped_file&) = delete;
  scoped_file(scoped_file&& other) = delete;
  scoped_file& operator=(scoped_file&& other) = delete;

  bool has_value() const noexcept;

  explicit operator bool() const noexcept;

  FILE* get() const noexcept;

  FILE* extract() noexcept;

  void reset() noexcept;

  scoped_file& write(std::string_view data) noexcept;
  scoped_file& flush() noexcept;
  scoped_file& sync() noexcept;

 private:
  FILE* file = nullptr;
};

}  // namespace tgbm::log::internal

#include <cstring>
#include <stdexcept>
#include <utility>

#include <cassert>

#include "tgbm_replace/scoped_file.hpp"

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>  // _get_osfhandle
#else
  #include <unistd.h>  // fileno, fsync, fdatasync
#endif

namespace tgbm::log::internal {

scoped_file::scoped_file(FILE* file) noexcept : file(file) {
  if (!file)
    std::terminate();
}

scoped_file::~scoped_file() {
  reset();
}

scoped_file::operator bool() const noexcept {
  return file != nullptr;
}

FILE* scoped_file::extract() noexcept {
  return std::exchange(file, nullptr);
}

FILE* scoped_file::get() const noexcept {
  return file;
}

void scoped_file::reset() noexcept {
  if (file != nullptr) {
    ::fclose(file);
    file = nullptr;
  }
}

bool scoped_file::has_value() const noexcept {
  return file != nullptr;
}

scoped_file& scoped_file::write(std::string_view data) noexcept {
  assert(has_value());
  if (data.empty())
    return *this;
  if (fwrite(data.data(), data.size(), 1, file) != 1) {
    std::terminate();
  }
  return *this;
}

scoped_file& scoped_file::flush() noexcept {
  assert(has_value());
  if (fflush(file) != 0) {
    std::terminate();
  }
  return *this;
}

scoped_file& scoped_file::sync() noexcept {
  flush();

#ifdef _WIN32
  int fd = _fileno(file);
  HANDLE h = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
  if (h == INVALID_HANDLE_VALUE || !FlushFileBuffers(h)) {
    std::terminate();
  }
#else
  int fd = fileno(file);
  if (fdatasync(fd) != 0) {  // or use fsync(fd)
    std::terminate();
  }
#endif

  return *this;
}

}  // namespace tgbm::log::internal

#pragma once

#include <string_view>
namespace bot::sql {

enum struct native_type {
  integer,
  text,
  floating,
  blob,
  null,
};

native_type from_sqlite_int(int type);

bool parse_native_type(std::string_view str, native_type& out);

}  // namespace bot::sql

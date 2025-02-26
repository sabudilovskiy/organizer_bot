cmake_minimum_required(VERSION 3.20)
project(bot)

include(cmake/deps.cmake)
include(cmake/migrations.cmake)

set(MIGRATIONS_SRC_DIR "${CMAKE_SOURCE_DIR}/migrations")
set(MIGRATIONS_BUILD_DIR "${CMAKE_BINARY_DIR}/generated/migrations")
set(MIGRATIONS_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
set(CMAKE_CXX_STANDARD 23)

generate_all_migration_files()

file(GLOB_RECURSE OBJS "src/*.cpp")
add_library(bot_objs ${OBJS})
target_include_directories(bot_objs PUBLIC include)
target_include_directories(bot_objs PUBLIC ${MIGRATIONS_INCLUDE_DIR})
target_link_libraries(bot_objs PUBLIC tgbmlib SQLiteCpp magic_enum::magic_enum scn::scn)

add_executable(bot_main main.cpp)
target_link_libraries(bot_main PUBLIC bot_objs)

target_include_directories(tgbmlib PUBLIC "${CMAKE_SOURCE_DIR}/logger")
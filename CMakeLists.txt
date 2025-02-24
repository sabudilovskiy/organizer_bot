cmake_minimum_required(VERSION 3.20)
project(bot)

include(cmake/deps.cmake)

target_include_directories(tgbmlib PUBLIC "${CMAKE_SOURCE_DIR}/logger")

get_target_property(TGBM_INCLUDE_DIRS tgbmlib INTERFACE_INCLUDE_DIRECTORIES)

file(GLOB_RECURSE OBJS "src/*.cpp")
add_library(bot_objs ${OBJS})
target_include_directories(bot_objs PUBLIC include)
target_link_libraries(bot_objs PUBLIC tgbmlib SQLiteCpp magic_enum::magic_enum)

add_executable(bot_main main.cpp)
target_link_libraries(bot_main PUBLIC bot_objs)

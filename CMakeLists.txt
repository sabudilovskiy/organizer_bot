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
target_link_libraries(bot_objs PUBLIC tgbmlib SQLiteCpp magic_enum::magic_enum scn::scn clinoklib)

add_executable(bot_main main.cpp)
set_target_properties(bot_main PROPERTIES 
    RUNTIME_OUTPUT_NAME organizer-bot)
target_link_libraries(bot_main PUBLIC bot_objs)
target_compile_definitions(bot_main PUBLIC store_main_args)
target_compile_definitions(bot_main PUBLIC program_options_file="options.def")
target_include_directories(bot_main PUBLIC "${CMAKE_SOURCE_DIR}/cli")

target_include_directories(tgbmlib PUBLIC "${CMAKE_SOURCE_DIR}/logger")
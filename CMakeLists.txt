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
add_library(${PROJECT_NAME}-objs ${OBJS})
target_include_directories(${PROJECT_NAME}-objs PUBLIC include)
target_include_directories(${PROJECT_NAME}-objs PUBLIC ${MIGRATIONS_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}-objs PUBLIC tgbmlib SQLiteCpp magic_enum::magic_enum scn::scn clinoklib)

add_executable(${PROJECT_NAME}-main main.cpp)
set_target_properties(${PROJECT_NAME}-main PROPERTIES 
    RUNTIME_OUTPUT_NAME organizer-bot)
target_link_libraries(${PROJECT_NAME}-main PUBLIC ${PROJECT_NAME}-objs)
target_compile_definitions(${PROJECT_NAME}-main PUBLIC store_main_args)
target_compile_definitions(${PROJECT_NAME}-main PUBLIC program_options_file="options.def")
target_include_directories(${PROJECT_NAME}-main PUBLIC "${CMAKE_SOURCE_DIR}/cli")

add_library(${PROJECT_NAME}-logger
    logger/src/backend.cpp
    logger/src/config.cpp
    logger/src/scoped_file.cpp
)
target_include_directories(${PROJECT_NAME}-logger PUBLIC "${CMAKE_SOURCE_DIR}/logger/include")
target_link_libraries(${PROJECT_NAME}-logger PUBLIC fmt::fmt)

target_link_libraries(tgbmlib PUBLIC ${PROJECT_NAME}-logger)

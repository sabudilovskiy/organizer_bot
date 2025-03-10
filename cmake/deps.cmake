include("${CMAKE_SOURCE_DIR}/cmake/get_cpm.cmake")

CPMAddPackage(
  NAME TGBM
  GIT_REPOSITORY https://github.com/bot-motherlib/TGBM
  GIT_TAG        a17f81e896cafe00306a67ffc5106db14106a4a3
)

set(SQLITECPP_RUN_CPPCHECK OFF)

CPMAddPackage(
  NAME SQLiteCpp
  GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp
  GIT_TAG        origin/master
)

CPMAddPackage(
  NAME magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum
  GIT_TAG        origin/master
)

SET(SCN_DISABLE_FAST_FLOAT ON)

CPMAddPackage(
  NAME scnlib
  GIT_REPOSITORY https://github.com/eliaskosunen/scnlib
  GIT_TAG        v4.0.1
)

CPMAddPackage(
  NAME clinok
  GIT_REPOSITORY https://github.com/kelbon/clinok
  GIT_TAG        d5637c0e21f88d82cf795b7a5b1a4da2f3d4b90f
)
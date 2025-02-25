include("${CMAKE_SOURCE_DIR}/cmake/get_cpm.cmake")

CPMAddPackage(
  NAME TGBM
  GIT_REPOSITORY https://github.com/bot-motherlib/TGBM
  GIT_TAG        v1.0.8
)

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
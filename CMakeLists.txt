############################################################################
# Global settings
#
# Version 3.5 does not support:
#
# - add_compile_definitions(_GNU_SOURCE "$<$<CONFIG:DEBUG>:DEBUG>")
#
# - target_compile_features(ccsvv PUBLIC c_std_11)
############################################################################

cmake_minimum_required (VERSION 3.5)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_compile_options(-std=c11 -Wall -Wextra -Wpedantic -Werror)

include_directories(inc)

############################################################################
# Project settings
############################################################################

project (ccsvv)

set(LIBS
  src/ncv_common.c
  src/ncv_ncurses.c
  src/ncv_parser.c
  src/ncv_table.c
  src/ncv_table_part.c
  src/ncv_corners.c
  src/ncv_field.c
  src/ncv_ui_loop.c
  src/ncv_win_header.c
  src/ncv_win_filter.c
  src/ncv_win_table.c
  src/ncv_win_footer.c
)

add_executable(ccsvv src/ncv_ccsvv.c ${LIBS})

target_compile_definitions(ccsvv PUBLIC _GNU_SOURCE "$<$<CONFIG:DEBUG>:DEBUG>")

target_link_libraries (ccsvv m formw ncursesw tinfo)

############################################################################
# Unit tests
############################################################################

enable_testing()

set(TEST_SRCS
  src/ut_field.c
  src/ut_parser.c
  src/ut_table.c
  src/ut_table_part.c
)

foreach(src_file ${TEST_SRCS})

  get_filename_component(exe_file ${src_file} NAME_WE)

  add_executable(${exe_file} ${src_file} src/ut_utils.c ${LIBS})

  target_compile_definitions(${exe_file} PUBLIC _GNU_SOURCE "$<$<CONFIG:DEBUG>:DEBUG>")
   
  target_link_libraries(${exe_file} m formw ncursesw tinfo)

  add_test(${exe_file} ${exe_file} ${CMAKE_SOURCE_DIR}/res)

endforeach()

cmake_minimum_required (VERSION 3.5)

project (ccsvv)

set(SOURCES
  src/ncv_ccsvv.c
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

add_executable(ccsvv ${SOURCES}) 

target_include_directories(ccsvv PUBLIC "${PROJECT_SOURCE_DIR}/inc")

target_link_libraries (ccsvv m formw ncursesw tinfo)

target_compile_definitions(ccsvv PUBLIC _GNU_SOURCE "$<$<CONFIG:DEBUG>:DEBUG>")

#### NOT SUPPORTED BY 3.5 ####
# target_compile_features(ccsvv PUBLIC c_std_11)

target_compile_options(ccsvv PUBLIC -std=c11 -Wall -Wextra -Wpedantic -Werror)

###############################################################################
# Global settings
#
# Version 3.5 does not support:
#
# - add_compile_definitions(_GNU_SOURCE "$<$<CONFIG:DEBUG>:DEBUG>")
#
# - target_compile_features(ccsvv PUBLIC c_std_11)
###############################################################################

cmake_minimum_required(VERSION 3.5)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_compile_options(-std=c11 -Wall -Wextra -Wpedantic -Werror -O2)

include_directories(inc)

###############################################################################
# Project settings
###############################################################################

project(ccsvv)

set(LIBS
  src/ncv_common.c
  src/ncv_ncurses.c
  src/ncv_parser.c
  src/ncv_table.c
  src/ncv_table_part.c
  src/ncv_corners.c
  src/ncv_field.c
  src/ncv_filter.c
  src/ncv_ui_loop.c
  src/ncv_forms.c
  src/ncv_popup.c
  src/ncv_wbuf.c
  src/ncv_win_header.c
  src/ncv_win_filter.c
  src/ncv_win_table.c
  src/ncv_win_footer.c
  src/ncv_win_help.c
)

add_executable(ccsvv src/ncv_ccsvv.c ${LIBS})

target_compile_definitions(ccsvv PUBLIC _GNU_SOURCE "$<$<CONFIG:DEBUG>:DEBUG>")

target_link_libraries(ccsvv m formw menuw ncursesw tinfo)

#
# Install executable with 755
#
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME} 
  DESTINATION /usr/bin
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE 
)

#
# Install license file
#
install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
  DESTINATION /usr/share/doc/${CMAKE_PROJECT_NAME}
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ 
)

###############################################################################
# Unit tests
###############################################################################

enable_testing()

set(TEST_SRCS
  src/ut_field.c
  src/ut_filter.c
  src/ut_parser.c
  src/ut_table.c
  src/ut_table_part.c
  src/ut_common.c
  src/ut_wbuf.c
)

foreach(src_file ${TEST_SRCS})

  get_filename_component(exe_file ${src_file} NAME_WE)

  add_executable(${exe_file} ${src_file} src/ut_utils.c ${LIBS})

  target_compile_definitions(${exe_file} PUBLIC _GNU_SOURCE "$<$<CONFIG:DEBUG>:DEBUG>")
   
  target_link_libraries(${exe_file} m formw menuw ncursesw tinfo)

  add_test(${exe_file} ${exe_file})

endforeach()

###############################################################################
# Debian package
###############################################################################

set(CPACK_GENERATOR "DEB")

#
# Architecture should be detected with: dpkg --print-architecture
#
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")

set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

#
# Package name out of the box is: ccsvv-0.2.0-Linux.deb
#
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

#
# Dependencies from my installation (ubuntu 18.0.4)
#
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>=2.27), libncursesw5 (>=6.1), libtinfo5 (>=6.1)")

#
# Various informations 
#
set(CPACK_PACKAGE_CONTACT "Dead End")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Curses based csv file viewer")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/dead-end/curses_csv_viewer")

set(CPACK_DEBIAN_PACKAGE_PRIORITY “optional”)
set(CPACK_DEBIAN_PACKAGE_SECTION  "Utilities")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

include(CPack)

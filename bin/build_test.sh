#/bin/sh

set -ue

bin_dir="bin"

################################################################################
# Error function.
################################################################################

do_exit () { 
   echo "ERROR: ${1}" 
   exit 1
}

################################################################################
# Ensure that we are in the right directory.
################################################################################

if [ ! -f makefile ] ; then
  do_exit "File not found in the current directory: makefile"
fi

if [ ! -f "${bin_dir}/cmake_build.sh" ] ; then
  do_exit "The build script does not exist: ${bin_dir}/cmake_build.sh"
fi

################################################################################
# Do builds.
################################################################################

make clean                || do_exit "make clean failed"

make DEBUG="true"         || do_exit "make debug failed"

make clean                || do_exit "make clean failed"

make                      || do_exit "make failed"

sh "${bin_dir}/cmake_build.sh" debug   || do_exit "cmake debug failed"

sh "${bin_dir}/cmake_build.sh" release || do_exit "cmake release failed"

exit 0


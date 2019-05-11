#/bin/sh

set -ue

###############################################################################
# error function
###############################################################################

do_exit () { 
   echo "ERROR: $1 failed" 
   exit 1
}

###############################################################################
# do builds
###############################################################################

make clean                || do_exit "make clean"

make DEBUG="-DDEBUG -g"   || do_exit "make debug"

make clean                || do_exit "make clean"

make                      || do_exit "make"

sh cmake_build.sh debug   || do_exit "cmake debug"

sh cmake_build.sh release || do_exit "cmake release"

exit 0

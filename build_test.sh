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

make                      || do_exit "make"

make clean                || do_exit "make clean"

make DEBUG="-DDEBUG -g"   || do_exit "make debug"

sh cmake_build.sh release || do_exit "cmake release"

sh cmake_build.sh debug   || do_exit "cmake debug"

exit 0

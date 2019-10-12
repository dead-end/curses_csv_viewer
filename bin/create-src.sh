#/bin/sh

################################################################################
# The program is call without arguments and builds a tar ball with the ccsvv 
# sources. The result is: ccsvv-{version}.tar.gz and the build should work with:
#
# > tar xvzf ccsvv-{version}.tar.gz
# > cd ccsvv-{version}/
# > make
# > make install
################################################################################

set -ue

################################################################################
# Error function.
################################################################################

do_exit() { 
  echo "ERROR: ${1}" 
  exit 1
}

################################################################################
# Ensure that we are in the correct directory.
################################################################################

if [ ! -d "build/" ] ; then
  do_exit "The directory: build/ does not exist!"
fi

################################################################################
# The load the version to the variable: version
################################################################################

version=$(sed -n 's/#define VERSION "\([^"]*\)"/\1/p' inc/ncv_common.h)

if [ "${version}" = "" ] ; then
  do_exit "No version found!"
fi

echo "Version: ${version}"

################################################################################
# Main program
################################################################################

#
# Remove old builds
#
if [ -d "build/ccsvv-${version}" ] ; then
  rm -rf "build/ccsvv-${version}/"
fi

#
# Create the build directory with the required content
#
build_root="build/ccsvv-${version}"

mkdir -p "${build_root}/build/" "${build_root}/tests/" || do_exit "Unable to create directories"

cp -r src/ inc/ man/ makefile LICENSE "${build_root}/" || do_exir "Unable to copy data"

#
# Create the tar ball
#
tar cvzf "build/ccsvv-${version}.tar.gz" "${build_root}/" || do_exit "Unable to create final tar"

exit 0


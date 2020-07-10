#/bin/sh

################################################################################
# The script builds a tar ball with the ccsvv sources or a debian .deb package.
################################################################################

set -ue

arch="amd64"

version_header="inc/ncv_common.h"

program="ccsvv"

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

version=$(sed -n 's/#define VERSION "\([^"]*\)"/\1/p' ${version_header})

if [ "${version}" = "" ] ; then
  do_exit "No version found!"
fi

echo "Version: ${version}"

################################################################################
# The function prints a usage / help message. If it is called with an argument
# this argument is interpreted as an error message and the function terminates
# the script with an exit code of 1. Otherwise the function terminates the 
# script normally.
################################################################################

usage() {
  echo "Usage: ${0} [help|src|deb"
  echo "  help : Prints this message"
  echo "  src  : Creates a tarball with the ${program} sources"
  echo "  deb  : Creates a debian package"

  if [ "${#}" != "0" ] ; then
    echo "\nERROR: ${1}"
    exit 1
  else
    exit 0
  fi
}

################################################################################
# The function builds a tarball with the program sources. The result is: 
#
#   ${program}-{version}.tar.gz 
#
# and the build should work with the tarball:
#
# > tar xvzf ${program}-{version}.tar.gz
# > cd ${program}-{version}/
# > make
# > make install
################################################################################

create_src() {

  #
  # Remove old directories
  #
  if [ -d "build/${program}-${version}" ] ; then
    rm -rf "build/${program}-${version}/"
  fi

  #
  # Create the root directory for the build
  #
  root_dir="build/${program}-${version}"

  #
  # Create the empty directories. The original directories may contain .md
  # files.
  #
  mkdir -p "${root_dir}/build/" "${root_dir}/tests/" || do_exit "Unable to create directories"

  cp -r src/ inc/ man/ makefile LICENSE "${root_dir}/" || do_exir "Unable to copy data"

  #
  # Create the tar ball
  #
  tar cvzf "build/${program}-${version}.tar.gz" "${root_dir}/" || do_exit "Unable to create final tar"
}

################################################################################
# The function builds a debian .deb package.
################################################################################

create_deb() {

  #
  # Remove old directories
  #
  if [ -d "build/${program}_${version}_${arch}" ] ; then
    rm -rf "build/${program}_${version}_${arch}/"
  fi

  #
  # Create the root directory for the build
  #
  root_dir="build/${program}_${version}_${arch}"

  mkdir -p "${root_dir}/DEBIAN" || do_exit "Unable to create dir: DEBIAN"

  make PREFIX="${root_dir}/usr" install || do_exit "Unable to call make"

  #
  # Get the dependencies
  #
  dependencies=$(sh bin/pkg-deps.sh no-debug ${program})

  echo "dependencies: ${dependencies}"

  #
  # Write the control file (${dependencies=} sets the default to "" if the
  # variable is not defined.)
  #
  cat << EOF > "${root_dir}/DEBIAN/control"
Package: ${program}
Version: ${version}
Priority: optional
Section: utils
Architecture: amd64
Homepage: https://github.com/dead-end/curses_csv_viewer
Depends: ${dependencies=}
Maintainer: dead-end
Description: Curses based csv file viewer
 Ccsvv is a viewer for csv (comma-separated values) data. It accepts input from 
 a file or from stdin. It supports different field separators, multi line 
 fields, escaping with " and filtering / sorting.
EOF

  #
  # Add md5sums
  #
  find "${root_dir}" -type f -not -path "*DEBIAN*" | \
    xargs md5sum | \
    sed "s#${root_dir}/##" > "${root_dir}/DEBIAN/md5sums"

  #
  # fakeroot sets permissions and owner:group
  #
  fakeroot dpkg-deb --build ${root_dir} || do_exit "dpkg-deb"
}

################################################################################
# Main program
################################################################################

if [ "${#}" != "1" ] ; then
  usage "Mode is missing"
fi

mode="${1}"

if [ "${mode}" = "help" ] ; then
  usage

elif [ "${mode}" = "src" ] ; then
  create_src

elif [ "${mode}" = "deb" ] ; then
  create_deb

else
  usage "Unknown mode: ${mode}"
fi

exit 0


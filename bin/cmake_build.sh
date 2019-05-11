#/bin/sh

###############################################################################
# Some saveguarding definitions. 
###############################################################################

set -ue

###############################################################################
# Ensure that we are in the correct directory.
###############################################################################

if [ ! -f CMakeLists.txt ] ; then
  echo "File not found in the current directory: CMakeLists.txt"
  exit 1
fi

###############################################################################
# Choose build mode "release" or "debug"
###############################################################################

BUILD_TYPE=Release

if [ "${#}" -eq "1" ] ; then

  if [ "${1}" = "debug" ] ; then
    BUILD_TYPE=Debug
    
  elif [ "${1}" != "release" ] ; then
    echo "ERROR: Unknown mode: $1 allowwd: 'release' or 'debug'"
    exit 1
  fi
  
fi

echo "Using build type: $BUILD_TYPE"

###############################################################################
# Create a clean build directory.
###############################################################################

build_dir="cmake-build"

if [ -d "${build_dir}" ] ; then
  echo "Removing: ${build_dir}"
  rm -rf "${build_dir}"
fi

echo "Creating: ${build_dir}"
mkdir "${build_dir}"

cd "${build_dir}"

###############################################################################
# Do the cmake build, test and packaging
###############################################################################

cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ..

make

make test

cpack ..

###############################################################################
# Print some informations about the newly build package.
###############################################################################

pkg_file=$(ls ccsvv*deb)

dpkg -I "${pkg_file}"

dpkg -c "${pkg_file}"

md5sum "${pkg_file}"

###############################################################################
# Create tar file with the executable
###############################################################################

tgz_file=$(echo "${pkg_file}" | sed 's#_amd64.deb#.tgz#')

tar -c --owner=dead --group=end -vzf "${tgz_file}" ccsvv

md5sum "${tgz_file}"

###############################################################################
# Go to the working directory and exit.
###############################################################################

cd ..

echo "Successfully build, test and package ccsvv :o)"

exit 0

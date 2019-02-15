#/bin/sh

###############################################################################
# Some saveguarding definitions. 
###############################################################################

set -ue

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

cmake -DCMAKE_BUILD_TYPE=Release ..
#cmake -DCMAKE_BUILD_TYPE=Debug ..

make

make test

cpack ..

###############################################################################
# Print some informations about the newly build package.
###############################################################################

file=$(ls ccsvv*deb)

dpkg -I "${file}"

dpkg -c "${file}"

###############################################################################
# Go to the working directory and exit.
###############################################################################

cd ..

echo "Successfully build, test and package ccsvv :o)"

exit 0

#/bin/sh

###############################################################################
#
###############################################################################

set -ue

DEBUG=true

build_dir="build"

exec="ccsvv"

version_header="inc/ncv_common.h"

arch="amd64"

################################################################################
# Error function.
################################################################################

do_exit() { 
  echo "ERROR: ${1}" 
  exit 1
}

###############################################################################
# A simple wrapper with error handling. 
###############################################################################

mk_dir() {
  dir="${1}"

  echo "Create directory: ${dir}"

  mkdir -p "${dir}" || do_exit "Unable to create dir: ${dir}"
}

###############################################################################
# A simple wrapper with error handling. 
###############################################################################

copy() {
  from="${1}"
  to="${2}"

  echo "Copy: ${from} to: ${to}"

  cp "${from}" "${to}" || do_exit "Unable to copy: ${from} to: ${to}"
}

###############################################################################
#
###############################################################################

if [ ! -d "${build_dir}" ] ; then
  do_exit "The directory: ${build_dir} does not exist!"
fi

if [ -d "${build_dir}/root" ] ; then
  rm -rf "${build_dir}/root"
fi

#
# Get program version from a header file
#
version=$(grep "#define VERSION " ${version_header} | sed 's#.* "## ; s#"##')

if [ "$version" = "" ] ; then
  do_exit "No version found in file: ${version_header}"
fi

echo "file: ${version_header} version: ${version}"

#
# Get thedependencies
#
dependencies=$(sh bin/pkg-deps.sh no-debug ${exec})

echo "dependencies: ${dependencies}"

deb="${exec}_${version}_${arch}"

root_dir="${build_dir}/root/${deb}"

mk_dir "${root_dir}/usr/bin"

mk_dir "${root_dir}/usr/share/doc/${exec}"

mk_dir "${root_dir}/DEBIAN"

copy "${exec}" "${root_dir}/usr/bin"

copy "LICENSE" "${root_dir}/usr/share/doc/${exec}"

sed "s#%(version)%#${version}# ; s#%(dependencies)%#${dependencies=}#" "pkgs/control" \
	> "${root_dir}/DEBIAN/control" || do_exit "Unable to replace version"

dpkg-deb --build ${root_dir} || do_exit "dpkg-deb"

exit 0


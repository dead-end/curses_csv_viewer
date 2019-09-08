#/bin/sh

################################################################################
#
################################################################################

set -u

CCSVV_VERSION=0.2.0

################################################################################
# Error function.
################################################################################

do_exit() {
   echo "ERROR: ${1}"
   exit 1
}

FAILED=1

check() {
  if [ "${1}" != "0" ] ; then
    echo "ERROR: ${2}"
    exit 1
  fi
}

################################################################################
# Main
################################################################################

#
# Ensure that the program is called from the root directory from the
# application.
#

#if [ ! -f "docker/bin/test_docker.sh" ] ; then
#  do_exit "The script is not called from the application root directory!"
#fi
test -f "docker/bin/test_docker.sh"
check "${?}" "The script is not called from the application root directory!"

if [ "${#}" = "1" ] ; then
  if [ "${1}" = "clean" ] ; then
    docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
    check "${?}" "Unable to cleanup"
  fi
fi

#
# Archlinux test
#
echo "TEST: archlinux test"

sudo docker build -t ccsvv_archlinux -f docker/archlinux.dockerfile docker/
check "${?}" "Unable to build ccsvv_archlinux"

docker run -it ccsvv_archlinux sh /tmp/curses_csv_viewer-master/bin/test_run.sh
check "${?}" "Unable to run ccsvv_archlinux"

#
# Fedora test
#
echo "TEST: fedora test"

sudo docker build -t ccsvv_fedora -f docker/fedora.dockerfile docker/
check "${?}" "Unable to build ccsvv_fedora"

docker run -it ccsvv_fedora sh /tmp/curses_csv_viewer-master/bin/test_run.sh
check "${?}" "Unable to run ccsvv_fedora"

#
# Ubuntu build test
#
echo "TEST: ubuntu build test"

sudo docker build -t ccsvv_deb_build --build-arg CCSVV_VERSION=${CCSVV_VERSION} -f docker/deb.build.dockerfile docker/
check "${?}" "Unable to build ccsvv_deb_build"

docker run -it ccsvv_deb_build sh /tmp/curses_csv_viewer-master/bin/test_run.sh
check "${?}" "Unable to run ccsvv_deb_build"

#
# Get the deb package from the docker image
#
docker cp $(docker ps -q -l):/tmp/curses_csv_viewer-master/cmake-build/ccsvv_${CCSVV_VERSION}_amd64.deb docker/tmp/
check "${?}" "Unable to extract .deb file from image"

#
# Ubuntu install test
#
echo "TEST: ubuntu install test"

sudo docker build -t ccsvv_deb_install --build-arg CCSVV_VERSION=${CCSVV_VERSION} -f docker/deb.install.dockerfile docker/ 
check "${?}" "Unable to build ccsvv_deb_install"

docker run -it ccsvv_deb_install sh /tmp/test_run.sh
check "${?}" "Unable to run ccsvv_deb_install"

# test "$?" = "0" || do_exit "Mist"
echo "Docker tests: OK"

exit 0


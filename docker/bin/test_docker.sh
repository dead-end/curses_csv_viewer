#/bin/sh

################################################################################
# The script triggers the docker related builds and tests. It has an optional
# argument "clean" that results in removing all docker containers and images
# before the start of the tests.
################################################################################

set -u

CCSVV_VERSION=0.2.0

################################################################################
# The function checks for errors.
################################################################################

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
test -f "docker/bin/test_docker.sh"
check "${?}" "The script is not called from the application root directory!"

#
# The script has an optional "clean" argument, that triggers the deletion of 
# docker images and containers.
#
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

docker run -it ccsvv_archlinux sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
check "${?}" "Unable to run ccsvv_archlinux"

#
# Fedora test
#
echo "TEST: fedora test"

sudo docker build -t ccsvv_fedora -f docker/fedora.dockerfile docker/
check "${?}" "Unable to build ccsvv_fedora"

docker run -it ccsvv_fedora sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
check "${?}" "Unable to run ccsvv_fedora"

#
# Build .deb package test 
#
echo "TEST: .deb build test"

sudo docker build -t ccsvv_deb_build --build-arg CCSVV_VERSION=${CCSVV_VERSION} -f docker/deb.build.dockerfile docker/
check "${?}" "Unable to build ccsvv_deb_build"

docker run -it ccsvv_deb_build sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
check "${?}" "Unable to run ccsvv_deb_build"

#
# Get the deb package from the docker image
#
docker cp $(docker ps -q -l):/tmp/curses_csv_viewer-master/cmake-build/ccsvv_${CCSVV_VERSION}_amd64.deb docker/tmp/
check "${?}" "Unable to extract .deb file from image"

#
# .deb install test
#
echo "TEST: .deb install test"

sudo docker build -t ccsvv_deb_install --build-arg CCSVV_VERSION=${CCSVV_VERSION} -f docker/deb.install.dockerfile docker/ 
check "${?}" "Unable to build ccsvv_deb_install"

docker run -it ccsvv_deb_install sh /tmp/test_run.sh
check "${?}" "Unable to run ccsvv_deb_install"

#
# ncurses source install test
#
echo "TEST: ncurses source install test"

sudo docker build -t ncurses_src -f docker/ncurses_src.dockerfile docker/
check "${?}" "Unable to build ncurses_src"

docker run -it ncurses_src sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
check "${?}" "Unable to run ncurses_src"

echo "Docker tests: OK"

exit 0


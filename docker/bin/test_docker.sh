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
# The function prints a usage / help message. If it is called with an argument
# this argument is interpreted as an error message and the function terminates
# the script with an exit code of 1. Otherwise the function terminates the 
# script normally.
################################################################################

usage() {
  echo "Usage: ${0} [clean|arch|fedora|deb|source|all]"  
  echo "  help     : Prints this message"
  echo "  clean    : Removes all docker images and containers"
  echo "  arch     : Runs a test on arch linux"
  echo "  fedora   : Runs a test on fedora linux"
  echo "  deb      : Builds a .deb package and tests the result"
  echo "  source   : Builds ccsvv from sources and tests the result"  
  echo "  all      : Runs all the tests above"

  if [ "${#}" != "0" ] ; then
    echo "\nERROR: ${1}"
    exit 1
  else
    exit 0
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

if [ "${#}" != "1" ] ; then
  usage "Mode is missing"
fi

mode="${1}"

#
# Print usage / help message
#
if [ "${mode}" = "help" ] ; then
  usage

#
# Remove all docker images and containers
#
elif [ "${mode}" = "clean" ] ; then
  docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
  check "${?}" "Unable to cleanup"


#
# Archlinux test
#
elif [ "${mode}" = "arch" -o "${mode}" = "all" ] ; then
  echo "TEST: archlinux test"

  sudo docker build -t ccsvv_archlinux -f docker/archlinux.dockerfile docker/
  check "${?}" "Unable to build ccsvv_archlinux"

  docker run -it ccsvv_archlinux sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
  check "${?}" "Unable to run ccsvv_archlinux"

#
# Fedora test
#
elif [ "${mode}" = "fedora" -o "${mode}" = "all" ] ; then
  echo "TEST: fedora test"

  sudo docker build -t ccsvv_fedora -f docker/fedora.dockerfile docker/
  check "${?}" "Unable to build ccsvv_fedora"

  docker run -it ccsvv_fedora sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
  check "${?}" "Unable to run ccsvv_fedora"


#
# Build .deb package test 
#
elif [ "${mode}" = "deb" -o "${mode}" = "all" ] ; then
  echo "TEST: .deb build test"

  sudo docker build -t ccsvv_deb_build --build-arg CCSVV_VERSION=${CCSVV_VERSION} -f docker/deb.build.dockerfile docker/
  check "${?}" "Unable to build ccsvv_deb_build"

  docker run -it ccsvv_deb_build sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
  check "${?}" "Unable to run ccsvv_deb_build"

  #
  # Get the deb package from the docker image
  #
  docker cp $(docker ps -q -l):/tmp/ccsvv_${CCSVV_VERSION}_amd64.deb docker/tmp/
  check "${?}" "Unable to extract .deb file from image"

  #
  # .deb install test
  #
  echo "TEST: .deb install test"

  sudo docker build -t ccsvv_deb_install --build-arg CCSVV_VERSION=${CCSVV_VERSION} -f docker/deb.install.dockerfile docker/ 
  check "${?}" "Unable to build ccsvv_deb_install"

  #
  # Run smoke test, check the man page and the copyright file
  #
  docker run -it ccsvv_deb_install sh -c "sh /tmp/test_run.sh && man --pager=cat ccsvv && cat /usr/share/doc/ccsvv/copyright"
  check "${?}" "Unable to run ccsvv_deb_install"

#
# ncurses source install test
#
elif [ "${mode}" = "source" -o "${mode}" = "all" ] ; then
  echo "TEST: ncurses source install test"

  sudo docker build -t ncurses_src -f docker/ncurses_src.dockerfile docker/
  check "${?}" "Unable to build ncurses_src"

  docker run -it ncurses_src sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
  check "${?}" "Unable to run ncurses_src"

else
  usage "Unknown mode: ${mode}"
fi

echo "Docker tests: OK"

exit 0


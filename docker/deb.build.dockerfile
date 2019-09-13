################################################################################
# File: deb.build.dockerfile
#
# Build the image:
#
#   sudo docker build -t ccsvv_deb_build -f docker/deb.build.dockerfile docker/
#
# Run test:
#
#   docker run -it ccsvv_deb_build sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
################################################################################

FROM ubuntu

#
# The ccsvv version should be set by the caller
#
ARG CCSVV_VERSION=0.2.0

#
# The ncurses major version necessary for the config script ncurses5-config or 
# ncurses6-config
#
ARG NCURSES_MAJOR=5

#
# Necessary for unit tests
#
ENV LANG=C.UTF-8

#
# Stop cmake on errors
#
ENV CTEST_OUTPUT_ON_FAILURE=1

#
# Download sources zipfile
#
ADD https://github.com/dead-end/curses_csv_viewer/archive/master.zip /tmp

#
# Install build tools and dependencies
#
RUN apt-get update && \
        apt-get install -y zip && \
        apt-get install -y cmake && \
        apt-get install -y libtinfo${NCURSES_MAJOR} && \
        apt-get install -y libncursesw${NCURSES_MAJOR} && \
        apt-get install -y libncursesw${NCURSES_MAJOR}-dev

#
# Set the directory for the build steps
#
WORKDIR /tmp

#
# Build ccsvv
#
RUN unzip master.zip && \
        cd curses_csv_viewer-master && \
        sh bin/cmake_build.sh && \
        apt-get install -y /tmp/curses_csv_viewer-master/cmake-build/ccsvv_${CCSVV_VERSION}_amd64.deb

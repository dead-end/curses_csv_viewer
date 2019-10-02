################################################################################
# File: ncurses_src.dockerfile
#
# Build the image:
#
#   sudo docker build -t ncurses_src -f ncurses_src.dockerfile docker/
#
# Run test:
#
#   docker run -it ncurses_src sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
################################################################################

FROM debian

#
# The ncurses version
#
ARG NCURSES_MAJOR=6
ARG NCURSES_MINOR=1
ARG NCURSES_VERSION=${NCURSES_MAJOR}.${NCURSES_MINOR}

#
# The installation prefix of ncurses
#
ARG PREFIX=/usr/local

#
# Necessary for unit tests
#
ENV LANG=C.UTF-8

#
# The library path has to be set to find the ncurses libraries.
#
ENV LD_LIBRARY_PATH=${PREFIX}/lib

#
# Set path so that test_run.sh finds ccsvv. This can be omitted if you install
# ccsvv with: make install. Installing ccsvv requires root rights which are not
# necessary present. This docker file tries to verify local installations with a
# prefix.
#
ENV PATH=$PATH:/tmp/curses_csv_viewer-master

#
# Install the build tools (procps contains the ps command, which is necessary
# for the smoke test script
#
RUN apt-get update && \
	apt-get install -y gcc && \
	apt-get install -y make && \
	apt-get install -y zip && \
	apt-get install -y procps

#
# Set the working directory for the installation
#
WORKDIR /tmp

#
# Download the ncurses sources
#
ADD https://invisible-mirror.net/archives/ncurses/ncurses-${NCURSES_VERSION}.tar.gz /tmp

#
# Build ccsvv
#
RUN tar xvzf ncurses-${NCURSES_VERSION}.tar.gz && \
	cd ncurses-${NCURSES_VERSION} && \
	./configure --prefix=${PREFIX} --enable-widec --with-shared --disable-leaks && \
	make && \
	make install

#
# Download the ccsvv sources
#
ADD https://github.com/dead-end/curses_csv_viewer/archive/master.zip /tmp

#
# Build ccsvv (see above: ENV PATH ...)
#
RUN unzip master.zip && \
	cd curses_csv_viewer-master && \
	make NCURSES_MAJOR=${NCURSES_MAJOR}

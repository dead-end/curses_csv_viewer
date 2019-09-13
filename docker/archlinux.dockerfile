################################################################################
# File: archlinux.dockerfile
#
# Build the image:
#
#   sudo docker build -t ccsvv_archlinux -f docker/archlinux.dockerfile docker/
#
# Run test:
#
#   docker run -it ccsvv_archlinux sh /tmp/curses_csv_viewer-master/bin/test_run.sh
################################################################################

FROM archlinux/base

#
# The ncurses major version necessary for the config script ncurses5-config or 
# ncurses6-config
#
ARG NCURSES_MAJOR=6

#
# A UTF8 locale is neccessary for unit tests. 'en_US.utf8' is the only
# installed.
#
ENV LANG=en_US.utf8

#
# Set path so that test_run.sh finds ccsvv.
#
ENV PATH=$PATH:/tmp/curses_csv_viewer-master

ADD https://github.com/dead-end/curses_csv_viewer/archive/master.zip /tmp

RUN pacman -Sy && \
	pacman -S --noconfirm unzip && \
	pacman -S --noconfirm gcc && \
	pacman -S --noconfirm make && \
	pacman -S --noconfirm ncurses && \
	pacman -S --noconfirm procps

#
# Set the directory for the build steps
#
WORKDIR /tmp

#
# Build ccsvv
#
RUN unzip master.zip && \
        cd curses_csv_viewer-master && \
        make NCURSES_MAJOR=${NCURSES_MAJOR}
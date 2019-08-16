# Docker

This document shows verious docker files to build and / or install *ccsvv* from a debian package
or from sources. The docker files can be seen as an installation instruction which can
be verified in a docker container.

## Useful docker commands

Search for an docker image:

```
docker search archlinux
```

Run the image *interactive* with a *tty*:

```
docker run -it archlinux/base
```

Build a docker image that will be tagged *image_tag* with a docker file *dockerfile* and a docker
context, which is a directory that contains mapped files and directories:

```
docker build -t image_tag -f dockerfile ctx

```

You can overwrite build arguments with: 

```
docker build ... --build-arg var_1=value_1 --build-arg var_2=value_2 ...
```

You can cleanup the docker images and containers with:

```
docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
```

## Installation with *.deb* packages

For the build of the docker image you need to set the docker context to the cmake build directory
(*cmake-build*), which contains the *.deb* file.

The docker file has variables for the OS, the *ccsvv* and the *ncurses* major version. The OS can
be any linux distribution that uses *.deb* packages. The default is *ubuntu*.

```
ARG DEB_OS=ubuntu

FROM ${DEB_OS}

ARG CCSVV_VERSION=0.2.0
ARG NCURSES_MAJOR=5

MAINTAINER dead-end

COPY ccsvv_${CCSVV_VERSION}_amd64.deb /tmp

RUN apt-get update && \
	apt-get install -y libtinfo${NCURSES_MAJOR} && \
	apt-get install -y libncursesw${NCURSES_MAJOR} && \
	apt-get install /tmp/ccsvv_${CCSVV_VERSION}_amd64.deb
```

The image for ubuntu and the start of the container can be done with the following
commands:

```
sudo docker build -t ccsvv_ubuntu_deb --build-arg DEB_OS=ubuntu cmake-build

docker run -it ccsvv_ubuntu_deb ccsvv -d : /etc/passwd
```

The same for debian:

```
sudo docker build -t ccsvv_debian_deb --build-arg DEB_OS=debian cmake-build

docker run -it ccsvv_debian_deb ccsvv -d : /etc/passwd
```

## Debian-like from sources

The following *Dockerfile* can be used to build *ncurses* and *ccsvv* from their sources.
The sources will be downloaded with wget, so no special docker contex is necessary. As an
OS again a linux that uses *.deb* packages are OK.

The build and installation of *ncurses* uses a prefix, so the *ncurses* can be installed
beside the system installation.

The build process is straightforward, first the build tools are installed, then *ncurses*
is downloaded, build and installed and finally *ccsvv* is downloaded, build and installed.

The three steps (install build tools, install *ncurses* and install *ccsvv*) have their own
*RUN* command, which creates a new layer.

```
ARG DEB_OS=ubuntu

FROM ${DEB_OS}

MAINTAINER dead-end

ARG NCURSES_MAJOR=6
ARG NCURSES_MINOR=1
ARG NCURSES_VERSION=${NCURSES_MAJOR}.${NCURSES_MINOR}
ARG PREFIX=/usr/local

RUN apt-get update && \
	apt-get install -y gcc && \
	apt-get install -y make && \
	apt-get install -y zip && \
	apt-get install -y wget

WORKDIR /tmp

RUN wget --no-verbose https://invisible-mirror.net/archives/ncurses/ncurses-${NCURSES_VERSION}.tar.gz && \
	tar xvzf ncurses-${NCURSES_VERSION}.tar.gz && \
	cd ncurses-${NCURSES_VERSION} && \
	./configure --prefix=${PREFIX} --enable-widec --with-shared --disable-leaks --includedir=${PREFIX}/include/ncursesw && \
	make && \
	make install

RUN wget --no-verbose https://github.com/dead-end/curses_csv_viewer/archive/master.zip && \
	unzip master.zip && \
	cd curses_csv_viewer-master && \
	make NCURSES_CONFIG=${PREFIX}/bin/ncursesw${NCURSES_MAJOR}-config
```

The build of the image and the running of the container is the following for debian:

```
sudo docker build -t ccsvv_debian_src --build-arg DEB_OS=debian .

docker run -it ccsvv_debian_src /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd

```

The same for ubuntu:

```
sudo docker build -t ccsvv_ubuntu_src --build-arg DEB_OS=ubuntu .

docker run -it ccsvv_ubuntu_src /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd
```

# Centos from sources

The Dockerfile for a centos build from sources.

```
FROM centos

MAINTAINER dead-end

ARG NCURSES_MAJOR=6
ARG NCURSES_MINOR=1
ARG NCURSES_VERSION=${NCURSES_MAJOR}.${NCURSES_MINOR}
ARG PREFIX=/usr/local

RUN yum -y update && \
	yum -y install gcc && \
	yum -y install make && \
	yum -y install zip && \
	yum -y install unzip && \
	yum -y install wget

WORKDIR /tmp

RUN wget --no-verbose https://invisible-mirror.net/archives/ncurses/ncurses-${NCURSES_VERSION}.tar.gz && \
	tar xvzf ncurses-${NCURSES_VERSION}.tar.gz && \
	cd ncurses-${NCURSES_VERSION} && \
	./configure --prefix=${PREFIX} --enable-widec --with-shared --disable-leaks --includedir=${PREFIX}/include/ncursesw && \
	make && \
	make install

RUN wget --no-verbose https://github.com/dead-end/curses_csv_viewer/archive/master.zip && \
	unzip master.zip && \
	cd curses_csv_viewer-master && \
	make NCURSES_CONFIG=${PREFIX}/bin/ncursesw${NCURSES_MAJOR}-config
```

# Archlinux from sources

These are the first steps ....

```
FROM archlinux/base

MAINTAINER dead-end

ARG NCURSES_MAJOR=6
ARG NCURSES_MINOR=1
ARG NCURSES_VERSION=${NCURSES_MAJOR}.${NCURSES_MINOR}
ARG PREFIX=/usr/local


RUN pacman -Sy && \
	pacman -S --noconfirm wget && \
	pacman -S --noconfirm zip && \
	pacman -S --noconfirm unzip && \
	pacman -S --noconfirm grep && \
	pacman -S --noconfirm awk && \
	pacman -S --noconfirm make && \
	pacman -S --noconfirm gcc

WORKDIR /tmp

RUN wget --no-verbose https://invisible-mirror.net/archives/ncurses/ncurses-${NCURSES_VERSION}.tar.gz && \
	tar xvzf ncurses-${NCURSES_VERSION}.tar.gz && \
	cd ncurses-${NCURSES_VERSION} && \
	./configure --prefix=${PREFIX} --enable-widec --with-shared --disable-leaks --includedir=${PREFIX}/include/ncursesw && \
	make && \
	make install

RUN wget --no-verbose https://github.com/dead-end/curses_csv_viewer/archive/master.zip && \
	unzip master.zip && \
	cd curses_csv_viewer-master && \
	make NCURSES_CONFIG=${PREFIX}/bin/ncursesw${NCURSES_MAJOR}-config
```
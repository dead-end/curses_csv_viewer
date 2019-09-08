# Docker

This document shows verious docker files to build and / or install *ccsvv* from a debian package
or from sources. The docker files can be seen as an installation instruction which can
be verified in a docker container.

## Useful docker commands

Search for a docker image:

```
docker search ubuntu
```

Run the image *interactive* with a *tty*:

```
docker run -it ubuntu
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

List the docker images and containers:

```
docker images

docker ps
```


## Installation with *.deb* packages

For the build of the docker image you need to set the docker context to the cmake build directory
(*cmake-build*), which contains the *.deb* file.

The docker file has variables for the OS and the *ccsvv* version. The OS can
be any linux distribution that uses *.deb* packages. The default is *ubuntu*.

```
ARG DEB_OS=ubuntu

FROM ${DEB_OS}

ARG CCSVV_VERSION=0.2.0

MAINTAINER dead-end

COPY ccsvv_${CCSVV_VERSION}_amd64.deb /tmp

RUN apt-get update && \
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

## Build from sources

Building *ccsvv* and *ncurses* from their sources is similar for the different linux operating systems. It
consists of two steps. In the first step an image for the os is created, that contains the tools to build
*ccsvv*. In the second step the compiling of the sources takes place. Let us start with the second step.

### Ubuntu build tool image

The second step is identical for all linux systems. The dockerfile requires a BUILD_OS parameter, which is
the tag of the image from the first step. It defines *ncurses* version informations and a PREFIX for the
installation. This is not strictly necessary. To use the PREFIX we have to export the LD_LIBRARY_PATH with
the lib directory from the PREFIX.

The build commands are straightforward.

* download the sources with wget
* extract the archive and cd to the directory
* call: ./configure, make, make install

```
# File: dockerfile.src

ARG BUILD_OS=

FROM ${BUILD_OS}

MAINTAINER dead-end

ARG NCURSES_MAJOR=6
ARG NCURSES_MINOR=1
ARG NCURSES_VERSION=${NCURSES_MAJOR}.${NCURSES_MINOR}
ARG PREFIX=/usr/local

ENV LD_LIBRARY_PATH=${PREFIX}/lib

WORKDIR /tmp

RUN wget --no-verbose https://invisible-mirror.net/archives/ncurses/ncurses-${NCURSES_VERSION}.tar.gz && \
	tar xvzf ncurses-${NCURSES_VERSION}.tar.gz && \
	cd ncurses-${NCURSES_VERSION} && \
	./configure --prefix=${PREFIX} --enable-widec --with-shared --disable-leaks && \
	make && \
	make install

RUN wget --no-verbose https://github.com/dead-end/curses_csv_viewer/archive/master.zip && \
	unzip master.zip && \
	cd curses_csv_viewer-master && \
	make NCURSES_CONFIG=${PREFIX}/bin/ncursesw${NCURSES_MAJOR}-config
```

### Ubuntu build tool image

The dockerfile to create the build tool image for ubuntu.

```
# File: dockerfile.ubuntu

FROM ubuntu

MAINTAINER dead-end

RUN apt-get update && \
	apt-get install -y gcc && \
	apt-get install -y make && \
	apt-get install -y zip && \
	apt-get install -y wget
```

The commands to build the images and run the container:

```
sudo docker build -t ccsvv_ubuntu -f dockerfile.ubuntu .

sudo docker build -t ccsvv_ubuntu_src --build-arg BUILD_OS=ccsvv_ubuntu -f dockerfile.src .

docker run -it ccsvv_ubuntu_src /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd
```

### Debian build tool image

The dockerfile to create the build tool image for debian.

```
# File: dockerfile.debian

FROM debian

MAINTAINER dead-end

RUN apt-get update && \
	apt-get install -y gcc && \
	apt-get install -y make && \
	apt-get install -y zip && \
	apt-get install -y wget
```

The commands to build the images and run the container:

```
sudo docker build -t ccsvv_debian -f dockerfile.debian .

sudo docker build -t ccsvv_debian_src --build-arg BUILD_OS=ccsvv_debian -f dockerfile.src .

docker run -it ccsvv_debian_src /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd
```

### Archlinux build tool image

The dockerfile to create the build tool image for archlinux.

```
# File: dockerfile.arch

FROM archlinux/base

MAINTAINER dead-end

RUN pacman -Sy && \
	pacman -S --noconfirm wget && \
	pacman -S --noconfirm unzip && \
	pacman -S --noconfirm grep && \
	pacman -S --noconfirm awk && \
	pacman -S --noconfirm make && \
	pacman -S --noconfirm gcc
```

The commands to build the images and run the container:

```
sudo docker build -t ccsvv_arch -f dockerfile.arch .

sudo docker build -t ccsvv_arch_src --build-arg BUILD_OS=ccsvv_arch -f dockerfile.src .

docker run -it ccsvv_arch_src /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd
```

### Centos build tool image

The dockerfile to create the build tool image for centos.

```
# File: dockerfile.centos

FROM centos

MAINTAINER dead-end

RUN yum -y update && \
	yum -y install gcc && \
	yum -y install make && \
	yum -y install unzip && \
	yum -y install wget
```

The commands to build the images and run the container:

```
sudo docker build -t ccsvv_centos -f dockerfile.centos .

sudo docker build -t ccsvv_centos_src --build-arg BUILD_OS=ccsvv_centos -f dockerfile.src .

docker run -it ccsvv_centos_src /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd
```

## Build and install *.deb* package on ubuntu

The following docker file creates a ubuntu container with build tools, compiles *ccsvv* and creates
a *.deb* package.

```dockerfile
################################################################################
# sudo docker build -t ccsvv_deb_build -f dockerfile.deb.build .
#
# docker run -it ccsvv_deb_build sh /tmp/curses_csv_viewer-master/bin/test_run.sh
################################################################################

FROM ubuntu

MAINTAINER dead-end

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

WORKDIR /tmp

#
# Build ccsvv
#
RUN unzip master.zip && \
        cd curses_csv_viewer-master && \
        sh bin/cmake_build.sh && \
        apt-get install -y /tmp/curses_csv_viewer-master/cmake-build/ccsvv_*_amd64.deb
```

With the following command, you can copy the newly created package from the container to the host filesystem. The
option `-q` is quiet and returns only numeric IDs and the option `-l` shows the latest created container.

```bash
docker cp $(docker ps -q -l):/tmp/curses_csv_viewer-master/cmake-build/ccsvv_0.2.0_amd64.deb .
```

The last docker file creates an ubuntu container and installes the newly created *.deb* package.

```dockerfile
################################################################################
# sudo docker build -t ccsvv_deb_install -f dockerfile.deb.install .
#
# docker run -it ccsvv_deb_install ccsvv -d : /etc/passwd
################################################################################

FROM ubuntu

ARG CCSVV_VERSION=0.2.0

MAINTAINER dead-end

COPY ccsvv_${CCSVV_VERSION}_amd64.deb /tmp

RUN apt-get update && \
	apt-get install /tmp/ccsvv_${CCSVV_VERSION}_amd64.deb
```

## Arch linux buid from sources

The following docker file provides an arch linux image with build tools and builds *ccsvv*.

```dockerfile
################################################################################
# File: dockerfile.arch
#
# Bild the image:
#
#   sudo docker build -t ccsvv_arch -f dockerfile.arch .
#
# Run test:
#
#   docker run -it ccsvv_arch sh /tmp/curses_csv_viewer-master/bin/test_run.sh
################################################################################

FROM archlinux/base

MAINTAINER dead-end

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
	pacman -S --noconfirm make && \
	pacman -S --noconfirm gcc && \
	pacman -S --noconfirm ncurses && \
	pacman -S --noconfirm procps

WORKDIR /tmp

#
# Build ccsvv
#
RUN unzip master.zip && \
        cd curses_csv_viewer-master && \
        make NCURSES_MAJOR=${NCURSES_MAJOR}
```

## Fedora buid from sources

The following docker file provides a fedora image with build tools and builds *ccsvv*.

```dockerfile
################################################################################
# File: dockerfile.fedora
#
# Bild the image:
#
#   sudo docker build -t ccsvv_fedora -f dockerfile.fedora .
#
# Run test:
#
#   docker run -it ccsvv_fedora sh /tmp/curses_csv_viewer-master/bin/test_run.sh
################################################################################

FROM fedora

MAINTAINER dead-end

ARG NCURSES_MAJOR=6

#
# A UTF8 locale is neccessary for unit tests. 'C.utf8' is the only
# installed.
#
ENV LANG=C.UTF-8

#
# Set path so that test_run.sh finds ccsvv.
#
ENV PATH=$PATH:/tmp/curses_csv_viewer-master

ADD https://github.com/dead-end/curses_csv_viewer/archive/master.zip /tmp

RUN yum -y update && \
	yum -y install gcc && \
	yum -y install make && \
	yum -y install unzip && \
	yum -y install ncurses-devel

WORKDIR /tmp

#
# Build ccsvv
#
RUN unzip master.zip && \
        cd curses_csv_viewer-master && \
        make NCURSES_MAJOR=${NCURSES_MAJOR}
```

## Centos buid from sources

The centos docker file is similar to the fedora docker file, but the wide character ncurses installation
in centos is broken.

```dockerfile
################################################################################
# File: dockerfile.centos
#
# Bild the image:
#
#   sudo docker build -t ccsvv_centos -f dockerfile.centos .
#
# Run test:
#
#   docker run -it ccsvv_centos sh /tmp/curses_csv_viewer-master/bin/test_run.sh
################################################################################

FROM centos

MAINTAINER dead-end

ARG NCURSES_MAJOR=5

#
# A UTF8 locale is neccessary for unit tests. 'C.utf8' is the only
# installed.
#
ENV LANG=en_GB.utf8

#
# Set path so that test_run.sh finds ccsvv.
#
ENV PATH=$PATH:/tmp/curses_csv_viewer-master

ADD https://github.com/dead-end/curses_csv_viewer/archive/master.zip /tmp

RUN yum -y update && \
	yum -y install gcc && \
	yum -y install make && \
	yum -y install unzip && \
	yum -y install ncurses-devel

WORKDIR /tmp

#
# Build ccsvv
#
RUN unzip master.zip
        cd curses_csv_viewer-master && \
        make NCURSES_MAJOR=${NCURSES_MAJOR}
```

 The header file does not contain all wide character related functions, so the build process
 fails:

```bash
 cc -c -o build/ncv_forms.o src/ncv_forms.c -std=c11 -O2 -D_GNU_SOURCE  -Wall -Wextra -Wpedantic -Werror -Iinc  -lncursesw -ltinfo -lformw -lmenuw -lm
src/ncv_forms.c: In function 'forms_driver':
src/ncv_forms.c:46:2: error: implicit declaration of function 'form_driver_w' [-Werror=implicit-function-declaration]
  const int result = form_driver_w(form, key_type, chr);
  ^
cc1: all warnings being treated as errors
make: *** [build/ncv_forms.o] Error 1
```

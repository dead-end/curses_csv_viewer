# Docker

This document shows verious docker files to build and / or install *ccsvv* from a debian package
or from sources. The docker files can be seen as an installation instruction which can
be verified in a docker container.

You can overwrite build arguments with: 

```
docker build --build-arg var_1=value_1 --build-arg var_2=value_2
```

You can cleanup the docker images and containes with:

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

## Ubuntu from sources

The following *Dockerfile* can be used to build *ncurses* and *ccsvv* from their sources.
The sources will be downloaded with wget, so no special docker contex is necessary.

```
FROM ubuntu 

MAINTAINER dead-end

ARG NCURSES_MAJOR=6
ARG NCURSES_MINOR=1
ARG NCURSES_VERSION=${NCURSES_MAJOR}.${NCURSES_MINOR}

RUN apt-get update && \
	apt-get install -y gcc && \
	apt-get install -y make && \
	apt-get install -y zip && \
	apt-get install -y wget 

WORKDIR /tmp

RUN wget --no-verbose https://invisible-mirror.net/archives/ncurses/ncurses-${NCURSES_VERSION}.tar.gz && \
	tar xvzf ncurses-${NCURSES_VERSION}.tar.gz && \
	cd ncurses-${NCURSES_VERSION} && \
	./configure --enable-widec --with-shared --disable-leaks --includedir=/usr/include/ncursesw && \
	make && \
	make install

RUN wget --no-verbose https://github.com/dead-end/curses_csv_viewer/archive/master.zip && \
	unzip master.zip && \
	cd curses_csv_viewer-master && \
	make NCURSES_CONFIG=ncursesw${NCURSES_MAJOR}-config
```

The you can build the image, run the container and do the cleanup with the following commands:

```
sudo docker build -t ccsvv_src .

docker run -it ccsvv_src /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd

docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
```

## Ubuntu from sources with prefix

The following *Dockerfile* can be used to build *ncurses* and *ccsvv* from their sources with a prefix directory.
The sources will be downloaded with wget, so no special docker contex is necessary.

```
FROM ubuntu 

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

The you can build the image, run the container and do the cleanup with the following commands:

```
sudo docker build -t ccsvv_pre .

docker run -it ccsvv_pre /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd

docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
```

# Centos from sources

The Dockerfile for a centos build from sources.

```
FROM centos

MAINTAINER dead-end

ARG NCURSES_MAJOR=6
ARG NCURSES_MINOR=1
ARG NCURSES_VERSION=${NCURSES_MAJOR}.${NCURSES_MINOR}

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
	./configure --enable-widec --with-shared --disable-leaks --includedir=/usr/include/ncursesw && \
	make && \
	make install

RUN wget --no-verbose https://github.com/dead-end/curses_csv_viewer/archive/master.zip && \
	unzip master.zip && \
	cd curses_csv_viewer-master && \
	make NCURSES_CONFIG=ncursesw${NCURSES_MAJOR}-config
```


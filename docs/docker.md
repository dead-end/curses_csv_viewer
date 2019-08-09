# Docker

## Ubuntu with deb

For the bild of the docker image you need to copy the .deb file from the cmake build (directory: cmake-build) 
to the context (here: ccsvv_deb) directory and add the *Dockerfile*. The version of *ccsv* and *ncurses* can
be set by build args if necessary.

```
FROM ubuntu 

MAINTAINER dead-end

ARG CCSVV_VERSION=0.2.0
ARG NCURSES_MAJOR=5

COPY ccsvv_${CCSVV_VERSION}_amd64.deb /tmp

RUN apt-get update && \
	apt-get install -y libtinfo${NCURSES_MAJOR} && \
	apt-get install -y libncursesw${NCURSES_MAJOR} && \
	apt-get install /tmp/ccsvv_${CCSVV_VERSION}_amd64.deb
```

The you can build the image, run the container and do the cleanup with the following commands:

```
# sudo docker build -t ccsvv_deb ccsvv_deb

# docker run -it ccsvv_deb ccsvv -d : /etc/passwd

# docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
```

## Ubuntu from sources

The following *Dockerfile* can be used to build *ncurses* and *ccsvv* from their sources.
The sources will be downloaded with wget, so no special docker contex is necessary.

```
FROM ubuntu 

MAINTAINER dead-end

ARG NCURSES_MAJOR=6
ARG NCURSES_VERSION=${NCURSES_MAJOR}.1

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
# sudo docker build -t ccsvv_src .

# docker run -it ccsvv_src /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd

# docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
```

## Ubuntu from sources with prefix

The following *Dockerfile* can be used to build *ncurses* and *ccsvv* from their sources with a prefix directory.
The sources will be downloaded with wget, so no special docker contex is necessary.

```
FROM ubuntu 

MAINTAINER dead-end

ARG NCURSES_MAJOR=6
ARG NCURSES_VERSION=${NCURSES_MAJOR}.1
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
# sudo docker build -t ccsvv_pre .

# docker run -it ccsvv_pre /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd

# docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
```

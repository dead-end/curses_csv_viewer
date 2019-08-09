# Docker

## Ubuntu with deb

For the bild of the docker image you need to copy the .deb file from the cmake build (directory: cmake-build) 
to the context (here: ccsvv_deb) directory and add the *Dockerfile*. The version of *ccsv* and *ncurses* can
be set by build args if necessary.

```
FROM ubuntu 

MAINTAINER dead-end

ARG CCSVV_VERSION=0.2.0
ARG NCURSES_VERSION=5

COPY ccsvv_${CCSVV_VERSION}_amd64.deb /tmp

RUN apt-get update && \
	apt-get install -y libtinfo${NCURSES_VERSION} && \
	apt-get install -y libncursesw${NCURSES_VERSION} && \
	apt-get install /tmp/ccsvv_${CCSVV_VERSION}_amd64.deb
```

The you can build the image, run the container and do the cleanup:

```
# sudo docker build -t ccsvv_deb --build-arg CCSVV_VERSION=0.2.0 --build-arg NCURSES_VERSION=5 ccsvv_deb

# docker run -it ccsvv_deb ccsvv -d : /etc/passwd

# docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
```

## Ubuntu from sources

The build from sources does not need a special context. It downloads the sources with wget.

```
FROM ubuntu 

MAINTAINER dead-end

ARG NCURSES_VERSION=6.1
ARG NCURSES_CONFIG_VERSION=6

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
	make NCURSES_CONFIG=ncursesw${NCURSES_CONFIG_VERSION}-config
```

The you can build the image, run the container and do the cleanup:

```
# sudo docker build -t ccsvv_src --build-arg NCURSES_VERSION=6.1 .

# docker run -it ccsvv_src /tmp/curses_csv_viewer-master/ccsvv -d : /etc/passwd

# docker rm $(docker ps -a -q) ; docker rmi $(docker images -q)
```
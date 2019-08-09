# Docker

## Ubuntu with deb

For the bild of the docker image you need to copy the .deb file from the cmake build to the context
directory and add the *Dockerfile*. The version of *ccsv* and *ncurses* can be set by build args if necessary.

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

# docker rm $(docker ps -a -q) && docker rmi $(docker images -q)
```

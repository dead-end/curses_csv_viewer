################################################################################
# File: deb.build.dockerfile
#
# Build the image:
#
#   sudo docker build -t ccsvv_deb_install -f docker/deb.install.dockerfile docker/
#
# Run test:
#
#   docker run -it ccsvv_deb_install sh /tmp/curses_csv_viewer-master/docker/bin/test_run.sh
################################################################################

FROM debian

#
# The ccsvv version should be set by the caller
#
ARG CCSVV_VERSION=0.2.0

#
# Copy the .deb package to the image
#
COPY tmp/ccsvv_${CCSVV_VERSION}_amd64.deb /tmp

#
# Copy the test script to the image
#
COPY bin/test_run.sh /tmp

#
# Install the .deb package
#
RUN apt-get update && \
	apt-get install -y man && \
	rm -f /etc/dpkg/dpkg.cfg.d/excludes && \
	apt-get install /tmp/ccsvv_${CCSVV_VERSION}_amd64.deb && \
	apt-get install -y procps

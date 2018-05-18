FROM ubuntu:xenial

MAINTAINER Andy Voigt <voigt-andy@hotmail.de>

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -qy apt-utils
RUN apt-get -qy install locales
RUN locale-gen --no-purge en_US.UTF-8
ENV LC_ALL en_US.UTF-8


RUN apt-get install -qy \
	git \
	build-essential \
	gawk \
	pkg-config \
	gettext \
	automake \
	autoconf \
	autopoint \
	libtool \
	bison \
	flex \
	zlib1g-dev \
	libgmp3-dev \
	libmpfr-dev \
	libmpc-dev \
	texinfo \
	mc \
	libncurses5-dev \
	nano \
	vim \
  	autopoint \
	gperf \
	python-docutils


RUN git clone https://bitbucket.org/padavan/rt-n56u.git /opt/rt-n56u

RUN cd /opt/rt-n56u/toolchain-mipsel && ./clean_sources && ./build_toolchain_3.4.x

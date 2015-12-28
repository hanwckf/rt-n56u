FROM ubuntu:trusty

MAINTAINER Andy Voigt <voigt-andy@hotmail.de>


RUN apt-get update && apt-get install -y \
	git \
	build-essential \
	gawk \
	pkg-config \
	gettext \
	automake \
	autoconf \
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
	vim 

RUN git clone https://bitbucket.org/padavan/rt-n56u.git /opt/rt-n56u

RUN cd /opt/rt-n56u/toolchain-mipsel && ./clean_sources && ./build_toolchain_3.4.x

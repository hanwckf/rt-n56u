FROM ubuntu:bionic

MAINTAINER hanwckf <hanwckf@vip.qq.com>

ENV DEBIAN_FRONTEND noninteractive

#RUN sed -i 's#http://archive.ubuntu.com#http://mirrors.huaweicloud.com#' /etc/apt/sources.list
#RUN sed -i 's#http://security.ubuntu.com#http://mirrors.huaweicloud.com#' /etc/apt/sources.list

RUN apt -y -q update && apt -y -q upgrade && \
	apt install -y -q unzip libtool-bin curl cmake gperf gawk flex bison htop \
		nano xxd fakeroot cpio git python-docutils gettext automake autopoint \
		texinfo build-essential help2man pkg-config zlib1g-dev libgmp3-dev libmpc-dev \
		libmpfr-dev libncurses5-dev libltdl-dev wget module-init-tools sudo locales vim && \
	rm -rf /var/cache/apt/

RUN echo 'en_US.UTF-8 UTF-8' > /etc/locale.gen && locale-gen

ENV LANG en_US.utf8

# See https://github.com/hanwckf/padavan-toolchain/releases
ADD mipsel-linux-uclibc.tar.xz /opt/rt-n56u/toolchain-mipsel/toolchain-3.4.x


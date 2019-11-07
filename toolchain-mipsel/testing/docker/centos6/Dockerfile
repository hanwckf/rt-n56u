FROM centos:6
ARG CTNG_UID
ARG CTNG_GID
RUN groupadd -g $CTNG_GID ctng
RUN useradd -d /home/ctng -m -g $CTNG_GID -u $CTNG_UID -s /bin/bash ctng
RUN yum install -y epel-release
RUN yum install -y autoconf gperf bison flex texinfo help2man gcc-c++ libtool libtool-bin patch \
    ncurses-devel python34-devel perl-Thread-Queue bzip2 git wget xz unzip
RUN wget -O /sbin/dumb-init https://github.com/Yelp/dumb-init/releases/download/v1.2.1/dumb-init_1.2.1_amd64
RUN chmod a+x /sbin/dumb-init
RUN echo 'export PATH=/opt/ctng/bin:$PATH' >> /etc/profile
# The limits in this file prevent su'ing to ctng user
RUN rm -f /etc/security/limits.d/90-nproc.conf
ENTRYPOINT [ "/sbin/dumb-init", "--" ]

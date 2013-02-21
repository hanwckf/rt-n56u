# This file is part of GNU Parted
# Copyright (C) 1999-2001, 2007-2012 Free Software Foundation, Inc.
#
# This file may be modified and/or distributed without restriction.

if COMPILE_FOR_S390
S390_SRCS = dasd.c fdasd.c vtoc.c
else
S390_SRCS =
endif

AM_CFLAGS = $(WARN_CFLAGS) $(WERROR_CFLAGS)

partedincludedir = \
  -I$(top_srcdir)/lib -I$(top_builddir)/include -I$(top_srcdir)/include \
  -I$(top_srcdir)/libparted
noinst_LTLIBRARIES    =	liblabels.la

liblabels_la_SOURCES = \
  $(S390_SRCS)	\
  aix.c		\
  bsd.c		\
  dos.c		\
  dvh.c		\
  dvh.h		\
  efi_crc32.c	\
  gpt.c		\
  loop.c	\
  mac.c		\
  misc.h	\
  pc98.c	\
  pt-common.h	\
  pt-tools.c	\
  pt-tools.h	\
  rdb.c		\
  sun.c

liblabels_la_LIBADD = $(OS_LIBS) $(INTLLIBS)

INCLUDES = $(partedincludedir) $(INTLINCS)

BUILT_SOURCES = pt-limit.c
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
liblabels_la_SOURCES += pt-limit.gperf
EXTRA_DIST = $(BUILT_SOURCES)

GPERF = gperf
GPERF_OPTIONS = \
  -C -N pt_limit_lookup -n -t -s 6 -k '*' --language=ANSI-C

pt-limit.c: pt-limit.gperf
	rm -f $@ $@-tmp
	$(GPERF) $(GPERF_OPTIONS) $< \
	  | perl -ne '/__GNUC_STDC_INLINE__/ and print "static\n"; print' \
	  > $@-tmp
	chmod a-w $@-tmp
	mv $@-tmp $@

# This file is part of GNU Parted
# Copyright (C) 1999-2001, 2007-2012 Free Software Foundation, Inc.
#
# This file may be modified and/or distributed without restriction.

TESTS = t1000-label.sh t2000-disk.sh t2100-zerolen.sh t3000-symlink.sh
EXTRA_DIST = $(TESTS)
check_PROGRAMS = label disk zerolen symlink
AM_CFLAGS = $(WARN_CFLAGS) $(WERROR_CFLAGS)

LDADD = \
  $(top_builddir)/libparted/libparted.la \
  $(CHECK_LIBS)

AM_CPPFLAGS = \
  $(CHECK_CFLAGS) \
  -I$(top_srcdir)/lib \
  -I$(top_builddir)/include \
  -I$(top_srcdir)/include

label_SOURCES = common.h common.c label.c
disk_SOURCES  = common.h common.c disk.c
zerolen_SOURCES = common.h common.c zerolen.c
symlink_SOURCES = common.h common.c symlink.c

# Arrange to symlink to tests/init.sh.
CLEANFILES = init.sh
.PHONY: prereq
prereq:
	$(AM_V_GEN)ln -sf $(abs_top_srcdir)/tests/init.sh .
$(TEST_LOGS): prereq

TESTS_ENVIRONMENT = \
  top_srcdir='$(top_srcdir)' \
  abs_top_srcdir='$(abs_top_srcdir)' \
  DYNAMIC_LOADING=$(DYNAMIC_LOADING) \
  ENABLE_DEVICE_MAPPER=$(ENABLE_DEVICE_MAPPER)

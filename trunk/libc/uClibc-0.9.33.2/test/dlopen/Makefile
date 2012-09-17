# uClibc dlopen tests
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

top_builddir=../../
include ../Rules.mak
-include Makefile.in
ifneq ($(HAVE_SHARED),y)
TESTS_DISABLED := test3
LDFLAGS_libtest.so := -lpthread
endif

include ../Test.mak

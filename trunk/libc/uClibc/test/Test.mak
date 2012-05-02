# Common makefile rules for tests
#
# Copyright (C) 2000,2001 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

include ../Rules.mak

U_TARGETS := $(TESTS)
G_TARGETS := $(patsubst %,%_glibc,$(U_TARGETS))
U_TARGETS += $(U_TESTS)
G_TARGETS += $(G_TESTS)
TARGETS    = $(U_TARGETS) $(G_TARGETS)

all: $(TARGETS)

$(TARGETS): Makefile $(TESTDIR)Rules.mak $(TESTDIR)Test.mak
$(U_TARGETS): $(patsubst %,%.c,$(U_TARGETS))
$(G_TARGETS): $(patsubst %_glibc,%.c,$(G_TARGETS))

$(U_TARGETS):
	-@ echo "----------------------------"
	-@ echo "Compiling $@ vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $@.c -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$$WRAPPER_$@ ./$@ $$OPTS_$@ ; \
		ret=$$? ; \
		test -z "$$RET_$@" && export RET_$@=0 ; \
		test $$ret -eq $$RET_$@
	-@ echo " "

$(G_TARGETS):
	-@ echo "----------------------------"
	-@ echo "Compiling $@ vs glibc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $(patsubst %_glibc,%,$@).c -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	$$WRAPPER_$(patsubst %_glibc,%,$@) ./$@ $$OPTS_$(patsubst %_glibc,%,$@) ; \
		ret=$$? ; \
		test -z "$$RET_$(patsubst %_glibc,%,$@)" && export RET_$(patsubst %_glibc,%,$@)=0 ; \
		test $$ret -eq $$RET_$(patsubst %_glibc,%,$@)
	-@ echo " "

clean:
	$(RM) *.[oa] *~ core $(TARGETS)

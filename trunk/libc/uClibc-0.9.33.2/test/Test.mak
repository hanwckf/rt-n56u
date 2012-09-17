# Common makefile rules for tests
#
# Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

ifeq ($(TESTS),)
TESTS := $(patsubst %.c,%,$(wildcard *.c))
endif
ifneq ($(TESTS_DISABLED),)
TESTS := $(filter-out $(TESTS_DISABLED),$(TESTS))
endif
ifeq ($(SHELL_TESTS),)
SHELL_TESTS := $(patsubst %.sh,shell_%,$(wildcard *.sh))
endif

ifneq ($(filter-out test,$(strip $(TESTS))),$(strip $(TESTS)))
$(error Sanity check: cannot have a test named "test.c")
endif

U_TARGETS := $(TESTS)
G_TARGETS := $(addsuffix _glibc,$(U_TARGETS))

ifneq ($(GLIBC_TESTS_DISABLED),)
G_TARGETS := $(filter-out $(GLIBC_TESTS_DISABLED),$(G_TARGETS))
endif

ifeq ($(GLIBC_ONLY),)
TARGETS   += $(U_TARGETS)
endif
ifeq ($(UCLIBC_ONLY),)
TARGETS   += $(G_TARGETS)
endif

CLEAN_TARGETS := $(U_TARGETS) $(G_TARGETS)
CLEAN_TARGETS += $(TESTS_DISABLED) $(addsuffix _glibc,$(TESTS_DISABLED)) $(GLIBC_TESTS_DISABLED)
COMPILE_TARGETS :=  $(TARGETS)
RUN_TARGETS := $(addsuffix .exe,$(TARGETS))
# provide build rules even for disabled tests:
U_TARGETS += $(TESTS_DISABLED)
G_TARGETS += $(addsuffix _glibc,$(TESTS_DISABLED)) $(GLIBC_TESTS_DISABLED)
TARGETS += $(SHELL_TESTS)
CFLAGS += $(CFLAGS_$(notdir $(CURDIR)))

define binary_name
$(patsubst %.exe,%,$@)
endef
define tst_src_name
$(patsubst %_glibc,%,$(binary_name))
endef

define diff_test
	$(Q)\
	for x in "$(binary_name).out" "$(tst_src_name).out" ; do \
		test -e "$$x.good" && $(do_showdiff) "$(binary_name).out" "$$x.good" && exec diff -u "$(binary_name).out" "$$x.good" ; \
	done ; \
	true
endef
define uclibc_glibc_diff_test
	$(Q)\
	test -z "$(DODIFF_$(tst_src_name))" && exec true ; \
	uclibc_out="$(binary_name).out" ; \
	glibc_out="$(tst_src_name).out" ; \
	$(do_showdiff) $$uclibc_out $$glibc_out ; \
	exec diff -u "$$uclibc_out" "$$glibc_out"
endef
define exec_test
	$(showtest)
	$(Q)\
	$(WRAPPER) $(WRAPPER_$(tst_src_name)) \
	./$(binary_name) $(OPTS) $(OPTS_$(tst_src_name)) > "$(binary_name).out" 2>&1 ; \
		ret=$$? ; \
		expected_ret="$(RET_$(tst_src_name))" ; \
		test -z "$$expected_ret" && export expected_ret=0 ; \
	if ! test $$ret -eq $$expected_ret ; then \
		echo "ret == $$ret ; expected_ret == $$expected_ret" ; \
		echo "The output of failed test is:"; \
		cat "$(binary_name).out"; \
		exit 1 ; \
	fi
	$(SCAT) "$(binary_name).out"
endef

test check all: run
run: $(RUN_TARGETS)
$(RUN_TARGETS):
	$(exec_test)
	$(diff_test)
ifeq ($(UCLIBC_ONLY),)
	$(uclibc_glibc_diff_test)
endif

compile: $(COMPILE_TARGETS)

G_TARGET_SRCS := $(addsuffix .c,$(G_TARGETS))
U_TARGET_SRCS := $(addsuffix .c,$(U_TARGETS))

$(MAKE_SRCS): Makefile $(TESTDIR)Makefile $(TESTDIR)Rules.mak $(TESTDIR)Test.mak

$(U_TARGETS): $(U_TARGET_SRCS) $(MAKE_SRCS)
	$(showlink)
	$(Q)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$(notdir $(CURDIR))) $(CFLAGS_$@) -c $@.c -o $@.o
	$(Q)$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LDFLAGS) $(LDFLAGS_$@)

$(G_TARGETS): $(U_TARGET_SRCS) $(MAKE_SRCS)
	$(showlink)
	$(Q)$(HOSTCC) $(HOST_CFLAGS) $(CFLAGS_$(notdir $(CURDIR))) $(CFLAGS_$(patsubst %_glibc,%,$@)) -c $(patsubst %_glibc,%,$@).c -o $@.o
	$(Q)$(HOSTCC) $(HOST_LDFLAGS) $@.o -o $@ $(EXTRA_LDFLAGS) $(LDFLAGS_$(patsubst %_glibc,%,$@))


shell_%:
	$(showtest)
	$(Q)$(if $(PRE_RUN_$(@)),$(PRE_RUN_$(@)))
	$(Q)$(SHELL) $(patsubst shell_%,%.sh,$(binary_name))
	$(Q)$(if $(POST_RUN_$(@)),$(POST_RUN_$(@)))

%.so: %.c
	$(showlink)
	$(Q)$(CC) \
		$(CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS_$(patsubst %_glibc,%,$@)) \
		-fPIC -shared $< -o $@ -Wl,-soname,$@ \
		$(LDFLAGS) $(EXTRA_LIBS) $(LDFLAGS_$(patsubst %_glibc,%,$@))

clean:
	$(showclean)
	$(Q)$(RM) *.a *.o *.so *~ core *.out *.gdb $(CLEAN_TARGETS) $(EXTRA_CLEAN)
	$(Q)$(RM_R) $(EXTRA_DIRS)

.PHONY: all check clean test run compile

XFAIL_TESTS = \
  t3200-type-change.sh

TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(SHELL)

TESTS = \
  help-version.sh \
  t0000-basic.sh \
  t0001-tiny.sh \
  t0010-script-no-ctrl-chars.sh \
  t0100-print.sh \
  t0101-print-empty.sh \
  t0200-gpt.sh \
  t0201-gpt.sh \
  t0202-gpt-pmbr.sh \
  t0203-gpt-tiny-device-abort.sh \
  t0203-gpt-shortened-device-primary-valid.sh \
  t0205-gpt-list-clobbers-pmbr.sh \
  t0206-gpt-print-with-corrupt-primary-clobbers-pmbr.sh \
  t0207-IEC-binary-notation.sh \
  t0208-mkpart-end-in-IEC.sh \
  t0209-gpt-pmbr_boot.sh \
  t0210-gpt-resized-partition-entry-array.sh \
  t0211-gpt-rewrite-header.sh \
  t0212-gpt-many-partitions.sh \
  t0220-gpt-msftres.sh \
  t0250-gpt.sh \
  t0280-gpt-corrupt.sh \
  t0300-dos-on-gpt.sh \
  t0400-loop-clobber-infloop.sh \
  t0500-dup-clobber.sh \
  t0501-duplicate.sh \
  t1100-busy-label.sh \
  t1101-busy-partition.sh \
  t1700-probe-fs.sh \
  t2200-dos-label-recog.sh \
  t2201-pc98-label-recog.sh \
  t2300-dos-label-extended-bootcode.sh \
  t2310-dos-extended-2-sector-min-offset.sh \
  t2400-dos-hfs-partition-type.sh \
  t2500-probe-corrupt-hfs.sh \
  t3000-resize-fs.sh \
  t3200-type-change.sh \
  t3300-palo-prep.sh \
  t3310-flags.sh \
  t3400-whole-disk-FAT-partition.sh \
  t4000-sun-raid-type.sh \
  t4001-sun-vtoc.sh \
  t4100-msdos-partition-limits.sh \
  t4100-dvh-partition-limits.sh \
  t4100-msdos-starting-sector.sh \
  t4200-partprobe.sh \
  t4300-nilfs2-tiny.sh \
  t5000-tags.sh \
  t6000-dm.sh \
  t6001-psep.sh \
  t6100-mdraid-partitions.sh \
  t7000-scripting.sh \
  t8000-loop.sh \
  t8001-loop-blkpg.sh \
  t9010-big-sector.sh \
  t9020-alignment.sh \
  t9021-maxima.sh \
  t9022-one-unit-snap.sh \
  t9023-value-lt-one.sh \
  t9030-align-check.sh \
  t9040-many-partitions.sh \
  t9041-undetected-in-use-16th-partition.sh \
  t9042-dos-partition-limit.sh \
  t9050-partition-table-types.sh

EXTRA_DIST = \
  $(TESTS) lvm-utils.sh t-local.sh t-lvm.sh \
  init.cfg init.sh t-lib-helpers.sh gpt-header-munge

check_PROGRAMS = print-align print-max dup-clobber duplicate fs-resize
fs_resize_LDADD = \
  $(top_builddir)/libparted/fs/libparted-fs-resize.la \
  $(top_builddir)/libparted/libparted.la

LDADD = \
  $(top_builddir)/libparted/libparted.la
AM_CPPFLAGS = \
  -I$(top_srcdir)/lib \
  -I$(top_builddir)/include \
  -I$(top_srcdir)/include
AM_CFLAGS = $(WARN_CFLAGS) $(WERROR_CFLAGS)

parted_dir = $(abs_top_builddir)/parted
pp_dir = $(abs_top_builddir)/partprobe
sep = $(PATH_SEPARATOR)

# Note that the first lines are statements.  They ensure that environment
# variables that can perturb tests are unset or set to expected values.
# The rest are envvar settings that propagate build-related Makefile
# variables to test scripts.
TESTS_ENVIRONMENT =				\
  tmp__=$$TMPDIR; test -d "$$tmp__" || tmp__=.;	\
  TMPDIR=$$tmp__; export TMPDIR;		\
  export					\
  abs_top_builddir='$(abs_top_builddir)'	\
  abs_top_srcdir='$(abs_top_srcdir)'		\
  abs_srcdir='$(abs_srcdir)'			\
  built_programs=parted				\
  srcdir='$(srcdir)'				\
  top_srcdir='$(top_srcdir)'			\
  AWK='$(AWK)'					\
  CC='$(CC)'					\
  MAKE=$(MAKE)					\
  PACKAGE_BUGREPORT='$(PACKAGE_BUGREPORT)'	\
  PACKAGE_VERSION=$(PACKAGE_VERSION)		\
  CONFIG_HEADER='$(abs_top_builddir)/lib/config.h' \
  ENABLE_DEVICE_MAPPER=$(ENABLE_DEVICE_MAPPER)	\
  PARTED_TEST_NAME=`basename '$(abs_srcdir)'`,`echo $$tst|sed 's,^\./,,;s,/,-,g'`\
  PERL='$(PERL)'				\
  PREFERABLY_POSIX_SHELL='$(PREFERABLY_POSIX_SHELL)' \
  REPLACE_GETCWD=$(REPLACE_GETCWD)		\
  PATH='$(abs_top_builddir)/parted$(PATH_SEPARATOR)'"$$PATH" \
  VERSION=$(VERSION)				\
  ; 9>&2

VERBOSE = yes

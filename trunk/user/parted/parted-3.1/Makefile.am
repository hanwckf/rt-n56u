ACLOCAL_AMFLAGS = -I m4
SUBDIRS = po lib include libparted parted partprobe

EXTRA_DIST =				\
  .version				\
  .prev-version				\
  BUGS					\
  libparted.pc.in			\
  parted.spec.in			\
  parted.spec				\
  scripts/data/abi/baseline_symbols.txt \
  scripts/extract_symvers		\
  cfg.mk				\
  dist-check.mk				\
  maint.mk				\
  GNUmakefile

aclocaldir=$(datadir)/aclocal

pcdir = $(libdir)/pkgconfig
pc_DATA = libparted.pc

# This is best not done via configure.ac, because automake's
# make distcheck target does not like auto-generated files
# being included in the distributed archive.
parted.spec: parted.spec.in
	sed 's/@''PACKAGE@/$(PACKAGE)/;s/@''VERSION@/$(VERSION)/' $< > $@-tmp
	mv $@-tmp $@
MAINTAINERCLEANFILES = parted.spec

.PHONY: root-check
root-check:
	sudo sh -c '$(MAKE) check; t=$$?; chown -R $(USER) .; exit $$t'

# Not everyone who runs "make distcheck" will want to or even
# be able to run the root tests.  If you want to skip them,
# run make like this: "make distcheck root-check="
root_check ?= root-check
distcheck-hook: $(root_check)
	$(MAKE) my-distcheck

### ABI Checking scripts ###

baseline_file = ${top_srcdir}/scripts/data/abi/baseline_symbols.txt
extract_symvers = $(top_srcdir)/scripts/extract_symvers

current_symbols.txt: ${extract_symvers}
	  -@(sh ${extract_symvers} libparted/.libs/libparted.so current_symbols.txt)

baseline_symbols:
	-@(output=${baseline_file}; \
	  if test ! -f $${output}; then \
	    echo "Baseline file doesn't exist."; \
	    echo "Try 'make new-abi-baseline' to create it."; \
	    exit 1; \
	  fi; true)

new-abi-baseline:
	-@$(mkinstalldirs) ${baseline_dir}
	-@(output=${baseline_file}; \
	  if test -f $${output}; then \
	    output=$${output}.new; \
	    t=`echo $${output} | sed 's=.*config/abi/=='`; \
	    echo "Baseline file already exists, writing to $${t} instead."; \
	  fi; \
	  sh ${extract_symvers} libparted/.libs/libparted.so $${output})

# Use 'new-abi-baseline' to create an initial symbol file.  Then run
# 'check-abi' to test for changes against that file.
check-abi: baseline_symbols current_symbols.txt
	@diff -u ${baseline_file} ./current_symbols.txt 2>&1 \
	 | tee libparted.abi-diff
	@test `wc -l < libparted.abi-diff` -eq 0 && exit 0 \
	 || echo "ABI has changed. Please, update you ABI package version." \
	 && exit 1

MOSTLYCLEANDIRS = m4

MAINTAINERCLEANFILES += \
  ABOUT-NLS     \
  ChangeLog     \
  INSTALL       \
  aclocal.m4    \
  configure

.PHONY: ss-1024 ss-2048 ss-4096
ss-1024 ss-2048 ss-4096:
	PARTED_SECTOR_SIZE=$(subst ss-,,$@) $(MAKE) check-recursive

# Run the regression test suite with different settings,
# to ensure it works with simulated partition sizes > 512.
.PHONY: check-other-sector_sizes
check-other-sector_sizes:
	$(MAKE) ss-1024
	$(MAKE) ss-2048
	$(MAKE) ss-4096

check: check-other-sector_sizes

# Arrange so that .tarball-version appears only in the distribution
# tarball, and never in a checked-out repository.
dist-hook: gen-ChangeLog
	echo $(VERSION) > $(distdir)/.tarball-version

gen_start_date = 2000-01-01
.PHONY: gen-ChangeLog
gen-ChangeLog:
	if test -d .git; then						\
	  $(top_srcdir)/build-aux/gitlog-to-changelog			\
	    --since=$(gen_start_date) > $(distdir)/cl-t;		\
	  rm -f $(distdir)/ChangeLog;					\
	  mv $(distdir)/cl-t $(distdir)/ChangeLog;			\
	fi

BUILT_SOURCES = .version
.version:
	echo $(VERSION) > $@-t && mv $@-t $@

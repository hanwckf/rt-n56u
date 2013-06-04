## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

include $(top_srcdir)/Make_global.am

if ! WITH_KBUILDDIR
KBUILD_OUTPUT=/lib/modules/`uname -r`/build
else
KBUILD_OUTPUT=$(KBUILDDIR)
endif
if ! WITH_MAXSETS
IP_SET_MAX=256
else
IP_SET_MAX=$(MAXSETS)
endif

SUBDIRS		= include/libipset lib src

sparse-check:
	$(MAKE) -C lib sparse-check
	$(MAKE) -C src sparse-check

modules_sparse:
if WITH_KMOD
	${MAKE} -C $(KBUILD_OUTPUT) M=$$PWD/kernel/net/netfilter \
			V=$V W=1 C=2 CF="-D__CHECK_ENDIAN__ -Wsparse-all" \
			KCFLAGS="-DPACKAGE_VERSION=$(PACKAGE_VERSION) -DCHECK_KCONFIG -Wextra" \
			IP_SET_MAX=$(IP_SET_MAX) KDIR=$$PWD/kernel modules
else
	@echo Skipping kernel modules due to --with-kmod=no
endif

modules:
if WITH_KMOD
	${MAKE} -C $(KBUILD_OUTPUT) M=$$PWD/kernel/net/netfilter V=$V \
			KCFLAGS="-DPACKAGE_VERSION=$(PACKAGE_VERSION)" \
			IP_SET_MAX=$(IP_SET_MAX) KDIR=$$PWD/kernel modules
else
	@echo Skipping kernel modules due to --with-kmod=no
endif

define DEPMOD_WARNING

!!! WARNING !!! WARNING !!! WARNING !!!

Your distribution seems to ignore the /lib/modules/<kernelrelease>/extra/
subdirectory, where the ipset kernel modules are installed.

Add the 'extra' directory to the search definition of your depmod
configuration (/etc/depmod.conf or /etc/depmod.d/) and re-run

	depmod <kernelrelease>

otherwise the ipset kernel modules in the extra subdir will be ignored.

endef
export DEPMOD_WARNING

modules_install:
if WITH_KMOD
	${MAKE} -C $(KBUILD_OUTPUT) M=$$PWD/kernel/net/netfilter \
			KDIR=$$PWD/kernel modules_install
	@modinfo ip_set_hash_ip | ${GREP} extra/ipset/ip_set_hash_ip >/dev/null || echo "$$DEPMOD_WARNING"
else
	@echo Skipping kernel modules due to --with-kmod=no
endif

modules_clean:
if WITH_KMOD
	${MAKE} -C $(KBUILD_OUTPUT) M=$$PWD/kernel/net/netfilter \
			KDIR=$$PWD/kernel clean
else
	@echo Skipping kernel modules due to --with-kmod=no
endif

update_includes:
	for x in ip_set.h ip_set_bitmap.h ip_set_hash.h ip_set_list.h; do \
	    cp kernel/include/uapi/linux/netfilter/ipset/$$x \
		include/libipset/linux_$$x; \
	done

update_utils:
	wget -O /tmp/ipset-bash-completion.tar.gz http://sourceforge.net/projects/ipset-bashcompl/files/latest/download
	cd utils/ipset_bash_completion; tar xz --strip-components=1 -f /tmp/ipset-bash-completion.tar.gz
	rm -f /tmp/ipset-bash-completion.tar.gz
	wget -O /tmp/ipset-list.tar.gz http://sourceforge.net/projects/ipset-list/files/latest/download
	cd utils/ipset_list; tar xz --strip-components=1 -f /tmp/ipset-list.tar.gz
	rm -f /tmp/ipset-list.tar.gz

tests:
	cd tests; ./runtest.sh

cleanup_dirs := . include/libipset lib src tests

tidy: distclean modules_clean
	rm -rf .deps $(foreach dir,$(cleanup_dirs),$(wildcard $(dir)/*~))
	rm -rf aclocal.m4 autom4te.cache 
	rm -rf config.* configure compile depcomp install-sh libtool ltmain.sh
	rm -rf Makefile Makefile.in lib/Makefile lib/Makefile.in src/Makefile src/Makefile.in
	rm -rf missing stamp-h1 m4/libtool.m4 m4/lt*.m4
	rm -rf lib/ipset_settype_check lib/types_init.c
	find . -name '*~' -delete

tarball:
	rm -Rf /tmp/ipset-${PACKAGE_VERSION};
	pushd ${top_srcdir} && git archive --prefix=ipset-${PACKAGE_VERSION}/ HEAD | tar -C /tmp -x && popd;
	pushd /tmp/ipset-${PACKAGE_VERSION} && ./autogen.sh && popd;
	tar -C /tmp -cjf ipset-${PACKAGE_VERSION}.tar.bz2 --owner=root --group=root ipset-${PACKAGE_VERSION}/;
	rm -Rf /tmp/ipset-${PACKAGE_VERSION};

.PHONY: modules modules_instal modules_clean update_includes tests tarball

DISTCHECK_CONFIGURE_FLAGS = --with-kmod=no

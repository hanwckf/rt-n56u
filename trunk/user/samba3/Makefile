srcdir=source

SMBCFLAGS = -Wall -O2 -ffunction-sections -fdata-sections
SMBLDFLAGS = -Wl,--gc-sections

ifneq ($(CONFIG_RT2880_FLASH_16M),y)
SMBCFLAGS += -DMAX_DEBUG_LEVEL="-1"
endif

all: config_test
	$(MAKE) -C $(srcdir) all

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	cd $(srcdir) && \
	SMB_BUILD_CC_NEGATIVE_ENUM_VALUES=yes \
	ac_cv_type_long_long=yes \
	fu_cv_sys_stat_statvfs64=yes \
	samba_cv_USE_SETEUID=yes \
	samba_cv_USE_SETREUID=yes \
	samba_cv_USE_SETRESUID=no \
	samba_cv_SIZEOF_INO_T=yes \
	samba_cv_SIZEOF_OFF_T=yes \
	samba_cv_have_longlong=yes \
	samba_cv_HAVE_C99_VSNPRINTF=yes \
	samba_cv_HAVE_INO64_T=yes \
	samba_cv_HAVE_OFF64_T=yes \
	samba_cv_HAVE_BROKEN_FCNTL64_LOCKS=no \
	samba_cv_HAVE_BROKEN_GETGROUPS=no \
	samba_cv_HAVE_BROKEN_READDIR=no \
	samba_cv_HAVE_BROKEN_LINUX_SENDFILE=no \
	samba_cv_HAVE_SENDFILE=yes \
	samba_cv_HAVE_FTRUNCATE_EXTEND=yes \
	samba_cv_HAVE_IFACE_AIX=no \
	samba_cv_HAVE_IFACE_IFCONF=yes \
	samba_cv_HAVE_IFACE_IFREQ=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_HAVE_UNSIGNED_CHAR=yes \
	samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
	samba_cv_HAVE_FCNTL_LOCK=yes \
	samba_cv_HAVE_STRUCT_FLOCK64=yes \
	samba_cv_HAVE_MMAP=no \
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=no \
	samba_cv_HAVE_KERNEL_SHARE_MODES=yes \
	samba_cv_HAVE_NATIVE_ICONV=no \
	samba_cv_REPLACE_INET_NTOA=no \
	samba_cv_LINUX_LFS_SUPPORT=yes \
	CPPFLAGS="-DNDEBUG -DSHMEM_SIZE=524288 -Dfcntl=fcntl64 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_LARGE_FILES=1" \
	CFLAGS="$(SMBCFLAGS)" LDFLAGS="$(SMBLDFLAGS)" \
	./configure \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) \
		--prefix=/var \
		--bindir=/bin \
		--sbindir=/sbin \
		--libdir=/lib \
		--localstatedir=/var \
		--with-configdir=/etc \
		--with-rootsbindir=/sbin \
		--with-piddir=/var/run \
		--with-privatedir=/etc/samba \
		--with-lockdir=/var/lock \
		--with-included-popt=no \
		--with-krb5=no \
		--with-syslog \
		--enable-shared \
		--enable-largefile \
		--disable-static \
		--disable-cups \
		--disable-iprint \
		--disable-pie \
		--disable-fam \
		--disable-dmalloc \
		--disable-krb5developer \
		--disable-developer \
		--disable-debug \
		--without-ads \
		--without-acl-support \
		--without-ldap \
		--without-cifsmount \
		--without-cifsupcall \
		--without-cluster-support \
		--without-utmp \
		--without-winbind \
		--without-krb5 \
		--without-quotas \
		--without-sys-quotas
	touch config_done
	mkdir -p $(srcdir)/bin

clean:
	$(MAKE) -C $(srcdir) distclean
	rm -f config_done
	rm -f smbmulti

distclean: clean
	@find $(srcdir) -name config.h | xargs rm -f
	@find $(srcdir) -name Makefile | xargs rm -f
	@find $(srcdir) -name config.status | xargs rm -f
	@find $(srcdir) -name config.cache | xargs rm -f

romfs:
	cp source/bin/smbmulti ./
	$(STRIP) smbmulti
	$(ROMFSINST) /sbin/smbmulti
	$(ROMFSINST) -s smbmulti /sbin/smbd
	$(ROMFSINST) -s smbmulti /sbin/nmbd
	$(ROMFSINST) -s /sbin/smbmulti /bin/smbpasswd

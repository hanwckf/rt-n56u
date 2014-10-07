SRC_NAME=source

SMBCFLAGS = -Wall -ffunction-sections -fdata-sections
SMBLDFLAGS = -Wl,--gc-sections

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMBD_SYSLOG),y)
SMBCFLAGS += -O2 -DMAX_DEBUG_LEVEL="0"
else
SMBCFLAGS += -Os -DMAX_DEBUG_LEVEL="-1"
endif

THISDIR = $(shell pwd)

all: config_test
	$(MAKE) -C $(SRC_NAME) all

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	cd $(SRC_NAME) && \
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
	samba_cv_HAVE_BROKEN_READDIR_NAME=no \
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
	samba_cv_REPLACE_READDIR=no \
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
		--libdir=/usr/lib/samba \
		--localstatedir=/var \
		--with-configdir=/etc \
		--with-rootsbindir=/sbin \
		--with-piddir=/var/run \
		--with-privatedir=/etc/samba \
		--with-lockdir=/var/locks \
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
	mkdir -p $(SRC_NAME)/bin

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

distclean: clean
	@find $(SRC_NAME) -name config.h | xargs rm -f
	@find $(SRC_NAME) -name Makefile | xargs rm -f
	@find $(SRC_NAME) -name config.status | xargs rm -f
	@find $(SRC_NAME) -name config.cache | xargs rm -f

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMBD),y)
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/bin/smbmulti /sbin/smbmulti
	$(ROMFSINST) -s smbmulti /sbin/smbd
	$(ROMFSINST) -s smbmulti /sbin/nmbd
	$(ROMFSINST) -s /sbin/smbmulti /bin/smbpasswd
	mkdir -p $(ROMFSDIR)/usr/lib/samba
	cp -f $(THISDIR)/$(SRC_NAME)/codepages/*.dat $(ROMFSDIR)/usr/lib/samba
else
ifeq ($(CONFIG_FIRMWARE_INCLUDE_WINS),y)
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/bin/nmbd /sbin/nmbd
endif
endif

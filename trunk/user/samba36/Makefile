SRC_NAME=source3

SMBCFLAGS = -Wall -ffunction-sections -fdata-sections $(CFLAGS)
SMBLDFLAGS = -Wl,--gc-sections $(LDFLAGS)

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMBD_SYSLOG),y)
SMBCFLAGS += -O2 -DMAX_DEBUG_LEVEL="0"
else
SMBCFLAGS += -Os -DMAX_DEBUG_LEVEL="-1"
endif

all: autoconf_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

autoconf_test:
	( if [ ! -f $(SRC_NAME)/configure ]; then \
		cd $(SRC_NAME) ; \
		./autogen.sh ; \
	fi )

configure:
	( cd $(SRC_NAME) && \
	libreplace_cv_HAVE_C99_VSNPRINTF=yes \
	libreplace_cv_HAVE_GETADDRINFO=no \
	libreplace_cv_HAVE_IFACE_IFCONF=yes \
	libreplace_cv_HAVE_IFACE_GETIFADDRS=yes \
	libreplace_cv_HAVE_SECURE_MKSTEMP=yes \
	libreplace_cv_REPLACE_INET_NTOA=no \
	libreplace_cv_HAVE_MREMAP=yes \
	libreplace_cv_HAVE_MMAP=yes \
	libreplace_cv_HAVE_OPEN_O_DIRECT=yes \
	libreplace_cv_HAVE_CLOSEFROM=yes \
	libreplace_cv_REPLACE_READDIR=no \
	libreplace_cv_REPLACE_INET_NTOA=no \
	libreplace_cv_HAVE_SOCK_SIN_LEN=no \
	libreplace_cv_READDIR_GETDIRENTRIES=no \
	libreplace_cv_HAVE_IPV6=no \
	LINUX_LFS_SUPPORT=yes \
	samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
	samba_cv_USE_SETEUID=yes \
	samba_cv_USE_SETREUID=yes \
	samba_cv_USE_SETRESUID=no \
	samba_cv_have_setreuid=yes \
	samba_cv_have_setresuid=yes \
	samba_cv_SIZEOF_INO_T=yes \
	samba_cv_SIZEOF_OFF_T=yes \
	samba_cv_have_longlong=yes \
	samba_cv_HAVE_C99_VSNPRINTF=yes \
	samba_cv_HAVE_INO64_T=yes \
	samba_cv_HAVE_OFF64_T=yes \
	samba_cv_HAVE_IPV6=no \
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
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes \
	samba_cv_HAVE_KERNEL_SHARE_MODES=yes \
	samba_cv_HAVE_NATIVE_ICONV=yes \
	samba_cv_LINUX_LFS_SUPPORT=yes \
	samba_cv_HAVE_WRFILE_KEYTAB=no \
	ac_cv_type_long_long=yes \
	ac_cv_func_sysctlbyname=no \
	ac_cv_header_libunwind_h=no \
	ac_cv_file__proc_sys_kernel_core_pattern=no \
	ac_cv_path_PYTHON="" \
	ac_cv_path_PYTHON_CONFIG="" \
	fu_cv_sys_stat_statvfs64=yes \
	PKG_CONFIG_PATH="$(STAGEDIR)/lib/pkgconfig" \
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
		--with-syslog \
		--enable-largefile \
		--disable-iprint \
		--disable-pie \
		--disable-cups \
		--disable-avahi \
		--disable-static \
		--disable-swat \
		--disable-shared-libs \
		--disable-relro \
		--disable-developer \
		--disable-krb5developer \
		--disable-debug \
		--without-ads \
		--without-krb5 \
		--without-ldap \
		--without-winbind \
		--without-pam \
		--without-sys-quotas \
		--without-quotas \
		--without-cluster-support \
		--without-acl-support \
		--without-automount \
		--with-sendfile-support \
		--without-libaddns \
		--without-libnetapi \
		--without-libsmbclient \
		--without-libsmbsharemodes \
		--without-libtdb \
		--disable-external-libtdb \
		--without-libtalloc \
		--disable-external-libtalloc \
		--without-libtevent \
		--disable-external-libtevent \
		--with-included-iniparser \
		--with-included-popt \
		--with-static-modules=nss_info_template,idmap_nss,pdb_tdbsam \
		--with-shared-modules=pdb_wbc_sam,auth_winbind,auth_wbc,auth_domain; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

distclean: clean
	@find $(SRC_NAME) -name config.h | xargs rm -f
	@find $(SRC_NAME) -name Makefile | xargs rm -f
	@find $(SRC_NAME) -name configure | xargs rm -f
	@find $(SRC_NAME) -name config.status | xargs rm -f
	@find $(SRC_NAME) -name config.cache | xargs rm -f

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMBD),y)
	$(ROMFSINST) $(SRC_NAME)/bin/samba_multicall /sbin/samba_multicall
	$(ROMFSINST) -s samba_multicall /sbin/smbd
	$(ROMFSINST) -s samba_multicall /sbin/nmbd
	$(ROMFSINST) -s /sbin/samba_multicall /bin/smbpasswd
	mkdir -p $(ROMFSDIR)/usr/lib/samba
	cp -f codepages/*.dat $(ROMFSDIR)/usr/lib/samba
else
ifeq ($(CONFIG_FIRMWARE_INCLUDE_WINS),y)
	$(ROMFSINST) $(SRC_NAME)/bin/nmbd /sbin/nmbd
endif
endif

SRC_DIR := samba-3.6.25
SRC_NAME := $(SRC_DIR)/source3
THISDIR := $(shell pwd)

SMBCFLAGS := $(CFLAGS) -ffunction-sections -fdata-sections
SMBLDFLAGS := $(LDFLAGS) -Wl,--gc-sections
SMBCPPFLAGS := -DNDEBUG -DSHMEM_SIZE=524288 -Dfcntl=fcntl64 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_LARGE_FILES=1
MAKE_FLAGS := $(MAKEFLAGS) DYNEXP= PICFLAG= MODULES=

ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMBD_SYSLOG),y)
SMBCFLAGS += -DMAX_DEBUG_LEVEL="0"
else
SMBCFLAGS += -DMAX_DEBUG_LEVEL="-1"
endif

all: config_test
	MAKEFLAGS="$(MAKE_FLAGS)" $(MAKE) -C $(SRC_NAME) basics
	MAKEFLAGS="$(MAKE_FLAGS)" $(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( test -d $(SRC_NAME) || { tar -xf $(SRC_DIR).tar.gz ; cat patches/*.patch | patch -d $(SRC_DIR) -r - -N -p1; }; \
	cd $(SRC_NAME) && \
	ac_cv_lib_attr_getxattr=no \
	ac_cv_search_getxattr=no \
	ac_cv_file__proc_sys_kernel_core_pattern=no \
	libreplace_cv_HAVE_C99_VSNPRINTF=yes \
	libreplace_cv_HAVE_GETADDRINFO=yes \
	libreplace_cv_HAVE_IFACE_IFCONF=yes \
	LINUX_LFS_SUPPORT=yes \
	samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
	samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
	samba_cv_HAVE_IFACE_IFCONF=yes \
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_HAVE_WRFILE_KEYTAB=no \
	samba_cv_USE_SETREUID=yes \
	samba_cv_USE_SETRESUID=yes \
	samba_cv_have_setreuid=yes \
	samba_cv_have_setresuid=yes \
	ac_cv_header_zlib_h=no \
	samba_cv_zlib_1_2_3=no \
	ac_cv_path_PYTHON="" \
	ac_cv_path_PYTHON_CONFIG="" \
	PKG_CONFIG_PATH="$(STAGEDIR)/lib/pkgconfig" \
	CPPFLAGS="$(SMBCPPFLAGS)" \
	CFLAGS="$(SMBCFLAGS)" LDFLAGS="$(SMBLDFLAGS)" \
	./configure \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) \
		--exec-prefix=/ \
		--prefix=/ \
		--enable-external-libtdb=no \
		--disable-external-libtalloc \
		--disable-dmalloc \
		--disable-iprint \
		--disable-fam \
		--disable-krb5developer \
		--disable-developer \
		--disable-debug \
		--disable-avahi \
		--disable-cups \
		--disable-pie \
		--disable-relro \
		--disable-static \
		--disable-swat \
		--disable-shared-libs \
		--with-libiconv="$(STAGEDIR)/lib" \
		--with-codepagedir=/usr/share/samba \
		--with-configdir=/etc \
		--with-included-iniparser \
		--with-included-popt=no \
		--with-rootsbindir=/sbin \
		--with-lockdir=/var/locks \
		--with-syslog \
		--with-nmbdsocketdir=/var/nmbd \
		--with-piddir=/var/run \
		--with-privatedir=/etc/samba \
		--with-sendfile-support \
		--without-acl-support \
		--without-cluster-support \
		--without-ads \
		--without-krb5 \
		--without-ldap \
		--without-pam \
		--without-utmp \
		--without-quotas \
		--without-sys-quotas \
		--without-winbind \
		--without-libtdb \
		--without-libtalloc \
		--without-libnetapi \
		--without-libsmbclient \
		--without-libsmbsharemodes \
		--without-libtevent \
		--without-libaddns \
		--with-shared-modules=pdb_tdbsam,pdb_wbc_sam,idmap_nss,nss_info_template,auth_winbind,auth_wbc,auth_domain; \
	mkdir -p bin; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ;
	rm -rf $(SRC_NAME)/bin
	rm -f config_done

distclean: clean
	@find $(SRC_NAME) -name config.h | xargs rm -f
	@find $(SRC_NAME) -name Makefile | xargs rm -f
	@find $(SRC_NAME) -name config.status | xargs rm -f
	@find $(SRC_NAME) -name config.cache | xargs rm -f

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_SMBD),y)
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/bin/samba_multicall /sbin/smbmulti
	$(ROMFSINST) -s smbmulti /sbin/smbd
	$(ROMFSINST) -s smbmulti /sbin/nmbd
	$(ROMFSINST) -s /sbin/smbmulti /bin/smbpasswd
	mkdir -p $(ROMFSDIR)/usr/share/samba
	cp -fP $(THISDIR)/$(SRC_DIR)/codepages/*.dat $(ROMFSDIR)/usr/share/samba
else
ifeq ($(CONFIG_FIRMWARE_INCLUDE_WINS),y)
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/bin/nmbd /sbin/nmbd
endif
endif

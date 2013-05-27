SRC_NAME1="nfs-utils-1.2.3"
SRC_NAME2="portmap_6.0"

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

PORTMAP_ENABLED=
ifeq ($(CONFIG_FIRMWARE_INCLUDE_NFSD),y)
PORTMAP_ENABLED=y
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_NFSC),y)
PORTMAP_ENABLED=y
endif

all: config_test
ifeq ($(CONFIG_FIRMWARE_INCLUDE_NFSD),y)
	$(MAKE) -C $(SRC_NAME1)
endif
ifeq ($(PORTMAP_ENABLED),y)
	$(MAKE) -C $(SRC_NAME2) NO_TCP_WRAPPER=1 DAEMON_UID=99 DAEMON_GID=99
endif

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	elif [ "$(CONFIG_FIRMWARE_INCLUDE_NFSD)" = "y" ] ; then \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME1) && \
	knfsd_cv_bsd_signals=no \
	./configure \
		--prefix=/ \
		--disable-ipv6 \
		--disable-tirpc \
		--disable-mount \
		--disable-libmount \
		--disable-nfsv4 \
		--disable-nfsv41 \
		--disable-uuid \
		--disable-gss \
		--without-krb5 \
		--without-tcp-wrappers \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME1)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME1) distclean ; \
	fi; \
	$(MAKE) -C $(SRC_NAME2) clean
	rm -f portmap
	rm -f exportfs
	rm -f rpc.nfsd
	rm -f rpc.mountd
	rm -f config_done

romfs:
ifeq ($(PORTMAP_ENABLED),y)
	cp $(SRC_NAME2)/portmap .
	$(ROMFSINST) /sbin/portmap
endif
ifeq ($(CONFIG_FIRMWARE_INCLUDE_NFSD),y)
	cp $(SRC_NAME1)/utils/exportfs/exportfs .
	cp $(SRC_NAME1)/utils/nfsd/nfsd ./rpc.nfsd
	cp $(SRC_NAME1)/utils/mountd/mountd ./rpc.mountd
	$(ROMFSINST) /usr/bin/nfsd.sh
	$(ROMFSINST) /sbin/exportfs
	$(ROMFSINST) /sbin/rpc.nfsd
	$(ROMFSINST) /sbin/rpc.mountd
endif

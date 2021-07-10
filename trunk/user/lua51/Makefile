SRC_NAME = lua-5.1.5
THISDIR = $(shell pwd)
TARGET_CFLAGS += -DLUA_USE_LINUX -fPIC -std=gnu99

all:
	$(MAKE) -C $(SRC_NAME) \
		CC="$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-gcc" \
		AR="$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-ar rcu" \
		RANLIB="$(CONFIG_CROSS_COMPILER_ROOT)/bin/mipsel-linux-uclibc-ranlib" \
		INSTALL_ROOT=/usr \
		CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
		PKG_VERSION=-"5.1.5" \
		MYLDFLAGS="$(TARGET_LDFLAGS)" \
		linux

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) clean ; \
	fi ; \
	rm -f config_done

romfs:
	cp -fP $(THISDIR)/lua-5.1.5/src/liblua.so.-5.1.5 $(ROMFSDIR)/lib
	cp -fP $(THISDIR)/lua-5.1.5/src/liblua.so $(ROMFSDIR)/lib
	$(ROMFSINST) $(THISDIR)/lua-5.1.5/src/lua /usr/bin/lua
	$(ROMFSINST) $(THISDIR)/lua/ /etc_ro/lua/
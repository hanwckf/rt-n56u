SRC_NAME=dropbear-2013.57

COPT=$(if $(CONFIG_FIRMWARE_INCLUDE_SFTP),-DSFTPSERVER_PATH=\"/usr/libexec/sftp-server\",-DSFTPSERVER_PATH=\"/opt/libexec/sftp-server\")
CFLAGS+=$(COPT) -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

all:	config_test
	$(MAKE) -C $(SRC_NAME) PROGRAMS="dropbear dbclient dropbearkey dropbearconvert scp" MULTI=1

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	./configure \
		--prefix=/usr \
		--disable-shadow \
		--disable-lastlog \
		--disable-utmp \
		--disable-utmpx \
		--disable-wtmp \
		--disable-wtmpx \
		--disable-zlib \
		--disable-loginfunc \
		--disable-pututline \
		--disable-pututxline \
		--enable-bundled-libtom \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f dropbearmulti
	rm -f config_done

romfs:
	cp $(SRC_NAME)/dropbearmulti .
	$(STRIP) dropbearmulti
	$(ROMFSINST) /usr/bin/dropbearmulti
	$(ROMFSINST) -p +x /usr/bin/sshd.sh
	$(ROMFSINST) -s dropbearmulti /usr/bin/ssh
	$(ROMFSINST) -s dropbearmulti /usr/bin/scp
	$(ROMFSINST) -s dropbearmulti /usr/bin/dropbearkey
	$(ROMFSINST) -s dropbearmulti /usr/bin/dropbearconvert
	$(ROMFSINST) -s /usr/bin/dropbearmulti /usr/sbin/dropbear

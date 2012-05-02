SRC_NAME=openssh-5.9p1

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

all:	config_test
	$(MAKE) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	LD="$(CC)" \
	./configure \
		--prefix=/usr \
		--sysconfdir=/etc/storage \
		--libexecdir=/usr/libexec \
		--with-pid-dir=/var/run \
		--with-random=/dev/urandom \
		--with-sandbox=no \
		--with-privsep-user=nobody \
		--without-pam \
		--without-selinux \
		--without-bsd-auth \
		--without-shadow \
		--without-kerberos5 \
		--without-x \
		--enable-shared \
		--disable-static \
		--disable-debug \
		--disable-strip \
		--disable-etc-default-login \
		--disable-lastlog \
		--disable-utmp \
		--disable-utmpx \
		--disable-wtmp \
		--disable-wtmpx \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean
	rm -f config_done
	rm -f scp
	rm -f ssh
	rm -f ssh-keygen
	rm -f sshd
	rm -f sftp-server

romfs:
	if [ "$(CONFIG_FIRMWARE_INCLUDE_OPENSSH)" = "y" ] ; then \
		cp $(SRC_NAME)/scp . ; \
		cp $(SRC_NAME)/ssh . ; \
		cp $(SRC_NAME)/ssh-keygen . ; \
		cp $(SRC_NAME)/sshd . ; \
		$(ROMFSINST) /usr/bin/scp ; \
		$(ROMFSINST) /usr/bin/ssh ; \
		$(ROMFSINST) /usr/bin/ssh-keygen ; \
		$(ROMFSINST) /usr/sbin/sshd ; \
		$(ROMFSINST) -p +x /usr/bin/sshd.sh ; \
	fi ; \
	cp $(SRC_NAME)/sftp-server .
	mkdir -p $(INSTALLDIR)/usr/libexec
	$(ROMFSINST) /usr/libexec/sftp-server



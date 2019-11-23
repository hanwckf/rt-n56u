SRC_NAME=openssh-8.1p1
SRC_URL=https://ftp.usa.openbsd.org/pub/OpenBSD/OpenSSH/portable/$(SRC_NAME).tar.gz

CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

THISDIR = $(shell pwd)

all: download_test extract_test config_test
	$(MAKE) -j$(HOST_NCPU) -C $(SRC_NAME)

download_test:
	( if [ ! -f $(SRC_NAME).tar.gz ]; then \
		wget -t5 --timeout=20 --no-check-certificate -O $(SRC_NAME).tar.gz $(SRC_URL); \
	fi )

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		tar xzf $(SRC_NAME).tar.gz; \
		patch -d $(SRC_NAME) -p1 -i ../openssh-orig.patch; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi )

configure:
	( cd $(SRC_NAME) ; \
	autoreconf -v; \
	LD="$(CC)" \
	./configure \
		--prefix=/usr \
		--sysconfdir=/etc/storage/openssh \
		--libexecdir=/usr/libexec \
		--with-pid-dir=/var/run \
		--with-privsep-path=/var/empty \
		--with-privsep-user=sshd \
		--with-sandbox=no \
		--without-pam \
		--without-selinux \
		--without-bsd-auth \
		--without-kerberos5 \
		--disable-strip \
		--disable-etc-default-login \
		--disable-lastlog \
		--disable-utmp \
		--disable-utmpx \
		--disable-wtmp \
		--disable-wtmpx \
		--disable-libutil \
		--host=$(HOST_TARGET) \
		--build=$(HOST_BUILD) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean ; \
	fi ; \
	rm -f config_done

romfs:
ifeq ($(CONFIG_FIRMWARE_INCLUDE_OPENSSH),y)
	mkdir -p $(INSTALLDIR)/etc_ro/ssh
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/moduli /etc_ro/ssh/moduli
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/libssh.so /lib/libssh.so
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/scp /usr/bin/scp
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/ssh /usr/bin/ssh
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/ssh-keygen /usr/bin/ssh-keygen
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/sshd /usr/sbin/sshd
	$(ROMFSINST) -p +x /usr/bin/sshd.sh
endif
	mkdir -p $(INSTALLDIR)/usr/libexec
	$(ROMFSINST) $(THISDIR)/$(SRC_NAME)/sftp-server /usr/libexec/sftp-server

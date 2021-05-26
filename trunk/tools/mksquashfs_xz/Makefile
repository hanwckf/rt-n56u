SRC_NAME1=squashfs-4.3
SRC_NAME2=xz-5.2.4

all: config_xz build_xz build_squashfs

config_xz:
	( if [ -f ./config_xz_done ]; then \
		echo "the same configuration"; \
	else \
		make configure_xz && touch config_xz_done; \
	fi )

configure_xz:
	( cd $(SRC_NAME2) ; \
	./autogen.sh ; \
	./configure \
		--disable-shared \
		--disable-werror \
		--disable-xz \
		--disable-xzdec \
		--disable-lzmadec \
		--disable-lzmainfo \
		CFLAGS="-std=c99" ; \
	)

build_xz:
	make -C $(SRC_NAME2)

build_squashfs:
	make -j$(HOST_NCPU) -C $(SRC_NAME1) XZUTILSDIR=$(shell pwd)/$(SRC_NAME2) ; \
	cp $(SRC_NAME1)/mksquashfs ./
	cp $(SRC_NAME1)/unsquashfs ./

clean:
	if [ -f $(SRC_NAME2)/Makefile ] ; then \
		make -C $(SRC_NAME2) distclean ; \
	fi
	make -C $(SRC_NAME1) clean
	rm -f config_xz_done
	rm -f mksquashfs unsquashfs

SRC_NAME = ffmpeg-0.11.2

DEMUXERS_CONFIG=--enable-demuxer=aac --enable-demuxer=asf --enable-demuxer=avi \
                --enable-demuxer=flac --enable-demuxer=h264 --enable-demuxer=m4v \
                --enable-demuxer=matroska --enable-demuxer=mov --enable-demuxer=mp3 \
                --enable-demuxer=mpegps --enable-demuxer=mpegts --enable-demuxer=mpegvideo \
                --enable-demuxer=ogg --enable-demuxer=wav

all: config_test
	$(MAKE) -C $(SRC_NAME)

config_test:
	( if [ -f ./config_done ]; then \
		echo "the same configuration"; \
	else \
		make configure && touch config_done; \
	fi ) 

configure:
	( cd $(SRC_NAME) ; \
	./configure \
		--prefix=/ \
		--cross-prefix=$(CROSS_COMPILE) \
		--enable-cross-compile \
		--arch=mips \
		--target-os=linux \
		--cc="$(CC)" \
		--as="$(AS)" \
		$(if $(CONFIG_LIB_BUILD_SHARED),--disable-static,--enable-static) \
		$(if $(CONFIG_LIB_BUILD_SHARED),--enable-shared,--disable-shared) \
		--enable-small \
		--enable-pthreads \
		--enable-gpl \
		--disable-zlib \
		--disable-debug \
		--disable-avdevice \
		--disable-network \
		--disable-swscale \
		--disable-swscale-alpha \
		--disable-ffmpeg \
		--disable-ffplay \
		--disable-ffprobe \
		--disable-ffserver \
		--disable-hwaccels \
		--disable-dxva2 \
		--disable-indevs \
		--disable-outdevs \
		--disable-devices \
		--disable-filters \
		--disable-encoders \
		--disable-muxers \
		--disable-decoders \
		--disable-demuxers \
		--disable-parsers \
		--disable-postproc \
		--disable-protocols \
		--disable-bsfs \
		--disable-everything \
		--enable-protocol=file \
		$(DEMUXERS_CONFIG) ; \
	)

clean:
	if [ -f $(SRC_NAME)/Makefile ] ; then \
		$(MAKE) -C $(SRC_NAME) distclean; \
	fi ; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install

romfs:
	if [ $(CONFIG_LIB_BUILD_SHARED) = "y" ] ; then \
		cp -fP $(DESTDIR)/lib/libavcodec.so* $(ROMFSDIR)/lib ; \
		cp -fP $(DESTDIR)/lib/libavformat.so* $(ROMFSDIR)/lib ; \
		cp -fP $(DESTDIR)/lib/libavutil.so* $(ROMFSDIR)/lib ; \
	fi


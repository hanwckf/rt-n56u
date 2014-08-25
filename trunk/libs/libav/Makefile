SRC_NAME_NEW = ffmpeg-0.11.x
SRC_NAME_OLD = ffmpeg-0.6.x

ifeq ($(CONFIG_FIRMWARE_INCLUDE_FFMPEG_NEW),y)
SRC_NAME = $(SRC_NAME_NEW)
PARAMS_CONFIG=--disable-swresample
else
SRC_NAME = $(SRC_NAME_OLD)
PARAMS_CONFIG=
endif

PARSERS_CONFIG=--enable-parser=aac --enable-parser=ac3 --enable-parser=mpegaudio

DEMUXERS_CONFIG=--enable-demuxer=aac --enable-demuxer=ac3 --enable-demuxer=asf \
                --enable-demuxer=avi --enable-demuxer=dts --enable-demuxer=eac3 \
                --enable-demuxer=flac --enable-demuxer=flv --enable-demuxer=h264 \
                --enable-demuxer=m4v --enable-demuxer=matroska --enable-demuxer=mov \
                --enable-demuxer=mp3 --enable-demuxer=mpegps --enable-demuxer=mpegts \
                --enable-demuxer=mpegvideo --enable-demuxer=ogg --enable-demuxer=wav

DECODERS_CONFIG=

ifeq ($(CONFIG_FIRMWARE_INCLUDE_FIREFLY),y)
DECODERS_CONFIG+=--enable-decoder=flac --enable-decoder=alac
endif

PROTOCOLS_CONFIG=--enable-protocol=file

CFLAGS += -D_LARGEFILE64_SOURCE -D_LFS64_LARGEFILE

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
		--prefix=$(STAGEDIR) \
		--cross-prefix=$(CROSS_COMPILE) \
		--enable-cross-compile \
		--arch=mips \
		--target-os=linux \
		--cc="$(CC)" \
		--as="$(AS)" \
		$(if $(LIBS_BUILD_SHARED),--disable-static,--enable-static) \
		$(if $(LIBS_BUILD_SHARED),--enable-shared,--disable-shared) \
		--enable-small \
		--enable-pthreads \
		--enable-gpl \
		--enable-zlib \
		--disable-doc \
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
		$(PARAMS_CONFIG) \
		$(PROTOCOLS_CONFIG) \
		$(PARSERS_CONFIG) \
		$(DECODERS_CONFIG) \
		$(DEMUXERS_CONFIG) ; \
	)

clean:
	if [ -f $(SRC_NAME_NEW)/config.mak ] ; then \
		$(MAKE) -C $(SRC_NAME_NEW) distclean; \
	fi ; \
	if [ -f $(SRC_NAME_OLD)/config.mak ] ; then \
		$(MAKE) -C $(SRC_NAME_OLD) distclean; \
	fi ; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install DESTDIR=""

romfs:
ifeq ($(LIBS_BUILD_SHARED),y)
	cp -fP $(DESTDIR)/lib/libavcodec.so* $(ROMFSDIR)/lib ;
	cp -fP $(DESTDIR)/lib/libavformat.so* $(ROMFSDIR)/lib ;
	cp -fP $(DESTDIR)/lib/libavutil.so* $(ROMFSDIR)/lib ;
endif


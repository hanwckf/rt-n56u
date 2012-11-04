SRC_NAME = ffmpeg-0.6.5

DEMUXERS_CONFIG=--disable-demuxer=amr --disable-demuxer=apc --disable-demuxer=ass \
                --disable-demuxer=bethsoftvid --disable-demuxer=bfi --disable-demuxer=c93 --disable-demuxer=daud \
                --disable-demuxer=dnxhd --disable-demuxer=dsicin --disable-demuxer=dxa --disable-demuxer=ffm \
                --disable-demuxer=gsm --disable-demuxer=gxf --disable-demuxer=idcin --disable-demuxer=iff \
                --disable-demuxer=image2 --disable-demuxer=image2pipe --disable-demuxer=ingenient --disable-demuxer=ipmovie \
                --disable-demuxer=lmlm4 --disable-demuxer=mm --disable-demuxer=mmf --disable-demuxer=msnwc_tcp --disable-demuxer=mtv \
                --disable-demuxer=mxf --disable-demuxer=nsv --disable-demuxer=nut --disable-demuxer=oma --disable-demuxer=pva \
                --disable-demuxer=rl2 --disable-demuxer=roq --disable-demuxer=rpl --disable-demuxer=segafilm \
                --disable-demuxer=shorten --disable-demuxer=siff --disable-demuxer=smacker --disable-demuxer=sol --disable-demuxer=str \
                --disable-demuxer=thp --disable-demuxer=tiertexseq --disable-demuxer=txd --disable-demuxer=vmd \
                --disable-demuxer=voc --disable-demuxer=wc3 --disable-demuxer=wsaud --disable-demuxer=wsvqa --disable-demuxer=xa \
                --disable-demuxer=yuv4mpegpipe --disable-demuxer=yop --disable-demuxer=swf --disable-demuxer=qcp --disable-demuxer=sox \
                --disable-demuxer=bink --disable-demuxer=iss --disable-demuxer=iv8 --disable-demuxer=rm --disable-demuxer=anm \
                --disable-demuxer=nc --disable-demuxer=nuv --disable-demuxer=aea --disable-demuxer=caf --disable-demuxer=flic \
                --disable-demuxer=fourxm --disable-demuxer=ea_cdata --disable-demuxer=cdg --disable-demuxer=mvi \
                --disable-demuxer=tmv --disable-demuxer=r3d --disable-demuxer=ea

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
	./configure \
		--prefix=/ \
		--cross-prefix=$(CROSS_COMPILE) \
		--enable-cross-compile \
		--arch=mips \
		--target-os=linux \
		$(if $(CONFIG_LIB_BUILD_SHARED),--disable-static,--enable-static) \
		$(if $(CONFIG_LIB_BUILD_SHARED),--enable-shared,--disable-shared) \
		$(if $(CONFIG_LIB_BUILD_SHARED),,--extra-cflags=-fvisibility=hidden) \
		--enable-small \
		--enable-pthreads \
		--disable-debug \
		--disable-avdevice \
		--disable-swscale \
		--disable-ffmpeg \
		--disable-ffplay \
		--disable-ffprobe \
		--disable-ffserver \
		--disable-swscale-alpha \
		--disable-encoders \
		--disable-decoders \
		--disable-hwaccels \
		--disable-bsfs \
		--disable-indevs \
		--disable-outdevs \
		--disable-devices \
		--disable-filters \
		--disable-muxers \
		--disable-parsers \
		--disable-protocols \
		--enable-protocol=file \
		--disable-aandct \
		--disable-dct \
		--disable-fft \
		--disable-golomb \
		--disable-lpc \
		--disable-mdct \
		--disable-rdft \
		--disable-vaapi \
		--disable-vdpau \
		--disable-dxva2 \
		--cc="$(CC)" \
		--as="$(AS)" \
		$(DEMUXERS_CONFIG) ; \
	)

clean:
	$(MAKE) -C $(SRC_NAME) distclean; \
	rm -f config_done

install:
	$(MAKE) -C $(SRC_NAME) install

romfs:
	if [ $(CONFIG_LIB_BUILD_SHARED) = "y" ] ; then \
		cp -fP $(DESTDIR)/lib/libavcodec.so* $(ROMFSDIR)/lib ; \
		cp -fP $(DESTDIR)/lib/libavformat.so* $(ROMFSDIR)/lib ; \
		cp -fP $(DESTDIR)/lib/libavutil.so* $(ROMFSDIR)/lib ; \
	fi


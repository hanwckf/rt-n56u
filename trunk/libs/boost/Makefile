SRC_NAME = boost_1_72_0
THISDIR = $(shell pwd)
CFLAGS := -O3 -ffunction-sections -fdata-sections
CXXFLAGS := -O3 -ffunction-sections -fdata-sections
LDFLAGS := -Wl,--gc-sections
Boost_libs := date_time,program_options,system

all: extract_test config_test
	( cd $(SRC_NAME); \
	./b2 -d 0 -j $(HOST_NCPU) toolset=gcc-cross link=static variant=release runtime-link=shared install ; \
	)

extract_test:
	( if [ ! -d $(SRC_NAME) ]; then \
		7z x -bsp0 $(SRC_NAME).7z ; \
		patch -d $(SRC_NAME) -p1 -i ../build.patch; \
	fi )

config_test:
	( if [ -f ./config_done ]; then \
		echo "config done"; \
	else \
	cd $(SRC_NAME); \
	./bootstrap.sh --with-libraries=$(Boost_libs) --prefix=$(STAGEDIR) ; \
	echo "using gcc : cross : $(CXX) : <compileflags>\"$(CPUFLAGS)\" <cxxflags>\"$(CXXFLAGS)\" <cflags>\"$(CFLAGS)\" <linkflags>\"$(LDFLAGS)\" ;" >> project-config.jam ; \
	touch ../config_done; \
	fi )

clean:
	rm -f config_done

romfs:
	cp -fP $(STAGEDIR)/lib/libboost_*.so* $(ROMFSDIR)/lib

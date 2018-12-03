###############################################
#          Compression build options          #
###############################################
#
#
############# Building gzip support ###########
#
# Gzip support is by default enabled, and the compression type default
# (COMP_DEFAULT) is gzip.
#
# If you don't want/need gzip support then comment out the GZIP SUPPORT line
# below, and change COMP_DEFAULT to one of the compression types you have
# selected.
#
# Obviously, you must select at least one of the available gzip, lzma, lzo
# compression types.
#
GZIP_SUPPORT = 1

########### Building XZ support #############
#
# LZMA2 compression.
#
# XZ Utils liblzma (http://tukaani.org/xz/) is supported
#
# To build using XZ Utils liblzma - install the library and uncomment
# the XZ_SUPPORT line below.
#
XZ_SUPPORT = 1


############ Building LZO support ##############
#
# The LZO library (http://www.oberhumer.com/opensource/lzo/) is supported.
#
# To build using the LZO library - install the library and uncomment the
# LZO_SUPPORT line below. If needed, uncomment and set LZO_DIR to the
# installation prefix.
#
#LZO_SUPPORT = 1
#LZO_DIR = /usr/local


########### Building LZ4 support #############
#
# Yann Collet's LZ4 tools are supported
# LZ4 homepage: http://fastcompression.blogspot.com/p/lz4.html
# LZ4 source repository: http://code.google.com/p/lz4
#
# To build configure the tools using cmake to build shared libraries,
# install and uncomment
# the LZ4_SUPPORT line below.
#
#LZ4_SUPPORT = 1


########### Building LZMA support #############
#
# LZMA1 compression.
#
# LZMA1 compression is deprecated, and the newer and better XZ (LZMA2)
# compression should be used in preference.
#
# Both XZ Utils liblzma (http://tukaani.org/xz/) and LZMA SDK
# (http://www.7-zip.org/sdk.html) are supported
#
# To build using XZ Utils liblzma - install the library and uncomment
# the LZMA_XZ_SUPPORT line below.
#
# To build using the LZMA SDK (4.65 used in development, other versions may
# work) - download and unpack it, uncomment and set LZMA_DIR to unpacked source,
# and uncomment the LZMA_SUPPORT line below.
#
#LZMA_XZ_SUPPORT = 1
#LZMA_SUPPORT = 1
#LZMA_DIR = ../../../../LZMA/lzma465

######## Specifying default compression ########
#
# The next line specifies which compression algorithm is used by default
# in Mksquashfs.  Obviously the compression algorithm must have been
# selected to be built
#
COMP_DEFAULT = xz

###############################################
#  Extended attribute (XATTRs) build options  #
###############################################
#
# Building XATTR support for Mksquashfs and Unsquashfs
#
# If your C library or build/target environment doesn't support XATTRs then
# comment out the next line to build Mksquashfs and Unsquashfs without XATTR
# support
XATTR_SUPPORT = 1

# Select whether you wish xattrs to be stored by Mksquashfs and extracted
# by Unsquashfs by default.  If selected users can disable xattr support by
# using the -no-xattrs option
#
# If unselected, Mksquashfs/Unsquashfs won't store and extract xattrs by
# default.  Users can enable xattrs by using the -xattrs option.
XATTR_DEFAULT = 1


###############################################
#        End of BUILD options section         #
###############################################

INCLUDEDIR = -I.
INSTALL_DIR = /usr/local/bin

MKSQUASHFS_OBJS = mksquashfs.o read_fs.o action.o swap.o pseudo.o compressor.o \
	sort.o progressbar.o read_file.o info.o restore.o process_fragments.o \
	caches-queues-lists.o

UNSQUASHFS_OBJS = unsquashfs.o unsquash-1.o unsquash-2.o unsquash-3.o \
	unsquash-4.o swap.o compressor.o unsquashfs_info.o

CFLAGS ?= -O2
CFLAGS += $(EXTRA_CFLAGS) $(INCLUDEDIR) -D_FILE_OFFSET_BITS=64 \
	-D_LARGEFILE_SOURCE -D_GNU_SOURCE -DCOMP_DEFAULT=\"$(COMP_DEFAULT)\" \
	-Wall

LIBS = -lpthread -lm
ifeq ($(GZIP_SUPPORT),1)
CFLAGS += -DGZIP_SUPPORT
MKSQUASHFS_OBJS += gzip_wrapper.o
UNSQUASHFS_OBJS += gzip_wrapper.o
LIBS += -lz
COMPRESSORS += gzip
endif

ifeq ($(LZMA_SUPPORT),1)
LZMA_OBJS = $(LZMA_DIR)/C/Alloc.o $(LZMA_DIR)/C/LzFind.o \
	$(LZMA_DIR)/C/LzmaDec.o $(LZMA_DIR)/C/LzmaEnc.o $(LZMA_DIR)/C/LzmaLib.o
INCLUDEDIR += -I$(LZMA_DIR)/C
CFLAGS += -DLZMA_SUPPORT
MKSQUASHFS_OBJS += lzma_wrapper.o $(LZMA_OBJS)
UNSQUASHFS_OBJS += lzma_wrapper.o $(LZMA_OBJS)
COMPRESSORS += lzma
endif

ifeq ($(LZMA_XZ_SUPPORT),1)
CFLAGS += -DLZMA_SUPPORT
MKSQUASHFS_OBJS += lzma_xz_wrapper.o
UNSQUASHFS_OBJS += lzma_xz_wrapper.o
LIBS += -llzma
COMPRESSORS += lzma
endif

ifeq ($(XZ_SUPPORT),1)
CFLAGS += -DXZ_SUPPORT
ifdef XZUTILSDIR
INCLUDEDIR += -I$(XZUTILSDIR)/src/liblzma/api
XZ_LIBDIR = -L$(XZUTILSDIR)/src/liblzma/.libs
LIBS += $(XZUTILSDIR)/src/liblzma/.libs/liblzma.a
endif
MKSQUASHFS_OBJS += xz_wrapper.o
UNSQUASHFS_OBJS += xz_wrapper.o
COMPRESSORS += xz
endif

ifeq ($(LZO_SUPPORT),1)
CFLAGS += -DLZO_SUPPORT
ifdef LZO_DIR
INCLUDEDIR += -I$(LZO_DIR)/include
LZO_LIBDIR = -L$(LZO_DIR)/lib
endif
MKSQUASHFS_OBJS += lzo_wrapper.o
UNSQUASHFS_OBJS += lzo_wrapper.o
LIBS += $(LZO_LIBDIR) -llzo2
COMPRESSORS += lzo
endif

ifeq ($(LZ4_SUPPORT),1)
CFLAGS += -DLZ4_SUPPORT
MKSQUASHFS_OBJS += lz4_wrapper.o
UNSQUASHFS_OBJS += lz4_wrapper.o
LIBS += -llz4
COMPRESSORS += lz4
endif

ifeq ($(XATTR_SUPPORT),1)
ifeq ($(XATTR_DEFAULT),1)
CFLAGS += -DXATTR_SUPPORT -DXATTR_DEFAULT
else
CFLAGS += -DXATTR_SUPPORT
endif
MKSQUASHFS_OBJS += xattr.o read_xattrs.o
UNSQUASHFS_OBJS += read_xattrs.o unsquashfs_xattr.o
endif

#
# If LZMA_SUPPORT is specified then LZMA_DIR must be specified too
#
ifeq ($(LZMA_SUPPORT),1)
ifndef LZMA_DIR
$(error "LZMA_SUPPORT requires LZMA_DIR to be also defined")
endif
endif

#
# Both LZMA_XZ_SUPPORT and LZMA_SUPPORT cannot be specified
#
ifeq ($(LZMA_XZ_SUPPORT),1)
ifeq ($(LZMA_SUPPORT),1)
$(error "Both LZMA_XZ_SUPPORT and LZMA_SUPPORT cannot be specified")
endif
endif

#
# At least one compressor must have been selected
#
ifndef COMPRESSORS
$(error "No compressor selected! Select one or more of GZIP, LZMA, XZ, LZO or \
	LZ4!")
endif

#
# COMP_DEFAULT must be a selected compressor
#
ifeq (, $(findstring $(COMP_DEFAULT), $(COMPRESSORS)))
$(error "COMP_DEFAULT is set to ${COMP_DEFAULT}, which  isn't selected to be \
	built!")
endif

.PHONY: all
all: mksquashfs unsquashfs

mksquashfs: $(MKSQUASHFS_OBJS)
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) $(MKSQUASHFS_OBJS) $(LIBS) -o $@

mksquashfs.o: Makefile mksquashfs.c squashfs_fs.h squashfs_swap.h mksquashfs.h \
	sort.h pseudo.h compressor.h xattr.h action.h error.h progressbar.h \
	info.h caches-queues-lists.h read_fs.h restore.h process_fragments.h 

read_fs.o: read_fs.c squashfs_fs.h squashfs_swap.h compressor.h xattr.h \
	error.h mksquashfs.h

sort.o: sort.c squashfs_fs.h mksquashfs.h sort.h error.h progressbar.h

swap.o: swap.c

pseudo.o: pseudo.c pseudo.h error.h progressbar.h

compressor.o: Makefile compressor.c compressor.h squashfs_fs.h

xattr.o: xattr.c squashfs_fs.h squashfs_swap.h mksquashfs.h xattr.h error.h \
	progressbar.h

read_xattrs.o: read_xattrs.c squashfs_fs.h squashfs_swap.h xattr.h error.h

action.o: action.c squashfs_fs.h mksquashfs.h action.h error.h

progressbar.o: progressbar.c error.h

read_file.o: read_file.c error.h

info.o: info.c squashfs_fs.h mksquashfs.h error.h progressbar.h \
	caches-queues-lists.h

restore.o: restore.c caches-queues-lists.h squashfs_fs.h mksquashfs.h error.h \
	progressbar.h info.h

process_fragments.o: process_fragments.c process_fragments.h

caches-queues-lists.o: caches-queues-lists.c error.h caches-queues-lists.h

gzip_wrapper.o: gzip_wrapper.c squashfs_fs.h gzip_wrapper.h compressor.h

lzma_wrapper.o: lzma_wrapper.c compressor.h squashfs_fs.h

lzma_xz_wrapper.o: lzma_xz_wrapper.c compressor.h squashfs_fs.h

lzo_wrapper.o: lzo_wrapper.c squashfs_fs.h lzo_wrapper.h compressor.h

lz4_wrapper.o: lz4_wrapper.c squashfs_fs.h lz4_wrapper.h compressor.h

xz_wrapper.o: xz_wrapper.c squashfs_fs.h xz_wrapper.h compressor.h

unsquashfs: $(UNSQUASHFS_OBJS)
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) $(UNSQUASHFS_OBJS) $(LIBS) -o $@

unsquashfs.o: unsquashfs.h unsquashfs.c squashfs_fs.h squashfs_swap.h \
	squashfs_compat.h xattr.h read_fs.h compressor.h

unsquash-1.o: unsquashfs.h unsquash-1.c squashfs_fs.h squashfs_compat.h

unsquash-2.o: unsquashfs.h unsquash-2.c squashfs_fs.h squashfs_compat.h

unsquash-3.o: unsquashfs.h unsquash-3.c squashfs_fs.h squashfs_compat.h

unsquash-4.o: unsquashfs.h unsquash-4.c squashfs_fs.h squashfs_swap.h \
	read_fs.h

unsquashfs_xattr.o: unsquashfs_xattr.c unsquashfs.h squashfs_fs.h xattr.h

unsquashfs_info.o: unsquashfs.h squashfs_fs.h

.PHONY: clean
clean:
	-rm -f *.o mksquashfs unsquashfs

.PHONY: install
install: mksquashfs unsquashfs
	mkdir -p $(INSTALL_DIR)
	cp mksquashfs $(INSTALL_DIR)
	cp unsquashfs $(INSTALL_DIR)

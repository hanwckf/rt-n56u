#
# Since version 1.6 the asynchronous mode has been
# disabled by default. To re-enable it uncomment the
# corresponding CFLAG.
#
CRYPTODEV_CFLAGS ?= #-DENABLE_ASYNC
KBUILD_CFLAGS += -I$(src) $(CRYPTODEV_CFLAGS)
VERSION = 1.8

cryptodev-objs = ioctl.o main.o cryptlib.o authenc.o zc.o util.o

obj-$(CONFIG_CRYPTO_DEV_LINUX) += cryptodev.o


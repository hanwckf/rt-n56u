# for SGlinux builds
-include $(ROOTDIR)/modules/.config

#
# You will need to point this at your Intel ixp425 includes,  this portion
# of the Makefile only really works under SGLinux with the appropriate libs
# installed.  They can be downloaded from http://www.snapgear.org/
#

IX_XSCALE_SW = drivers/ixp400/ixp400_xscale_sw
OSAL_DIR     = drivers/ixp400/ixp_osal

ifdef IX_XSCALE_SW
IXP_CFLAGS = \
	-I$(ROOTDIR)/. \
	-I$(IX_XSCALE_SW)/src/include \
	-I$(OSAL_DIR)/ \
	-I$(OSAL_DIR)/os/linux/include/ \
	-I$(OSAL_DIR)/os/linux/include/modules/ \
	-I$(OSAL_DIR)/os/linux/include/modules/ioMem/ \
	-I$(OSAL_DIR)/os/linux/include/modules/core/ \
	-I$(OSAL_DIR)/os/linux/include/modules/bufferMgt/ \
	-I$(OSAL_DIR)/os/linux/include/core/  \
	-I$(OSAL_DIR)/os/linux/include/platforms/ \
	-I$(OSAL_DIR)/os/linux/include/platforms/ixp400/ \
	-I$(OSAL_DIR)/os/linux/include/platforms/ixp400/ixp425 \
	-I$(OSAL_DIR)/os/linux/include/platforms/ixp400/ixp465 \
	-I$(OSAL_DIR)/os/linux/include/core/ \
	-I$(OSAL_DIR)/include/ \
	-I$(OSAL_DIR)/include/modules/ \
	-I$(OSAL_DIR)/include/modules/bufferMgt/ \
	-I$(OSAL_DIR)/include/modules/ioMem/ \
	-I$(OSAL_DIR)/include/modules/core/ \
	-I$(OSAL_DIR)/include/platforms/ \
	-I$(OSAL_DIR)/include/platforms/ixp400/ \
	-I$(OSAL_DIR)/include/platforms/ixp400/ixp425 \
	-I$(OSAL_DIR)/include/platforms/ixp400/ixp465 \
	-I$(IX_XSCALE_SW)/src/linux \
	-DUSE_IXP4XX_CRYPTO
endif
ifdef CONFIG_IXP400_LIB_1_4
IXP_CFLAGS   = \
	-I$(ROOTDIR)/. \
	-I$(ROOTDIR)/modules/ixp425/ixp400-1.4/ixp400_xscale_sw/src/include \
	-I$(ROOTDIR)/modules/ixp425/ixp400-1.4/ixp400_xscale_sw/src/linux \
	-DUSE_IXP4XX_CRYPTO
endif
ifndef IXPDIR
IXPDIR = ixp-version-is-not-supported
endif

ifeq ($(CONFIG_MACH_IXDP465),y)
IXP_CFLAGS += -D__ixp46X
else
IXP_CFLAGS += -D__ixp42X
endif

obj-$(CONFIG_OCF_IXP4XX) += ixp4xx.o

obj ?= .
EXTRA_CFLAGS += $(IXP_CFLAGS) -I$(obj)/.. -I$(obj)/.

ifdef TOPDIR
-include $(TOPDIR)/Rules.make
endif


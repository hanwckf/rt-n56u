#############################################################
# Makefile for WSC upnp module
#############################################################

ifndef ROOTDIR
ROOTDIR = ../..
export ROOTDIR
endif

WIRELESS_H_INCLUDE_PATH = $(LINUXINCDIR)

PWD = $(shell pwd)

LIBUPNP_SRC_PATH = ./libupnp-1.3.1
LIBUPNP_LIB_PATH = $(LIBUPNP_SRC_PATH)

INCLUDE_PATH += -I. -I$(LIBUPNP_SRC_PATH) -I$(LIBUPNP_SRC_PATH)/upnp/inc -I$(LIBUPNP_SRC_PATH)/upnp/src/inc
INCLUDE_PATH += -I$(LIBUPNP_SRC_PATH)/ixml/inc -I$(LIBUPNP_SRC_PATH)/ixml/src/inc  -I$(LIBUPNP_SRC_PATH)/threadutil/inc
INCLUDE_PATH += -I$(WIRELESS_H_INCLUDE_PATH)

INCLUDE_LIBS := -L$(LIBUPNP_LIB_PATH)/upnp -lupnp -L$(LIBUPNP_LIB_PATH)/ixml -lixml -L$(LIBUPNP_LIB_PATH)/threadutil -lthreadutil -lpthread

OBJS = wsc_main.o wsc_common.o wsc_ioctl.o wsc_netlink.o wsc_upnp_device.o wsc_upnp_cp.o sample_util.o wsc_msg.o

EXEC = wscd

CFLAGS += -DMULTIPLE_CARD_SUPPORT
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections

all: $(EXEC)

# Compile build-in libupnp-1.3.1
$(EXEC): libupnp $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(INCLUDE_LIBS) $(LDFLAGS)
	$(STRIP) -s $(EXEC)

# Compile build-in libupnp-1.3.1
libupnp:
	make -C $(LIBUPNP_SRC_PATH)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -c $<

clean:
	make -C $(LIBUPNP_SRC_PATH) clean
	rm -rf *.o *~ *.bak $(EXEC) ; \

cleanall: clean
	rm -f *.a
	rm -f libupnp-1.3.1/Makefile

romfs:
	$(ROMFSINST) wscd /bin/wscd
	$(ROMFSINST) xml/WFADeviceDesc.xml /etc_ro/xml/WFADeviceDesc.xml
	$(ROMFSINST) xml/WFAWLANConfigSCPD.xml /etc_ro/xml/WFAWLANConfigSCPD.xml


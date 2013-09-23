CFLAGS += -fPIC
CFLAGS += -Wall -I. -I./include
CFLAGS += $(if $(CONFIG_FIRMWARE_INCLUDE_OPENVPN),-DAPP_OPENVPN,)
CFLAGS += $(if $(CONFIG_RAETH_GMAC2),,-DUSE_SINGLE_MAC)
CFLAGS += $(if $(CONFIG_RT3352_INIC_MII),-DUSE_RT3352_MII,)
CFLAGS += $(if $(CONFIG_USB_XHCI_HCD),-DUSE_USB3,)
CFLAGS += $(if $(CONFIG_IPV6),-DUSE_IPV6,)

ifeq ($(CONFIG_FIRMWARE_INCLUDE_HTTPS),y)
CFLAGS += -DSUPPORT_HTTPS
endif

ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N56U")
CFLAGS += -DBOARD_N56U
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N65U")
CFLAGS += -DBOARD_N65U
CFLAGS += -DUSE_IWPRIV_RADIO_5G
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"SWR1100")
CFLAGS += -DBOARD_SWR1100
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"BN750DB")
CFLAGS += -DBOARD_BN750DB
endif

LDFLAGS += -L.

OBJS := shutils.o netutils.o defaults.o nvram_linux.o notify_rc.o bin_sem_asus.o flash_mtd.o pids.o

all: libshared.so

libshared.so: $(OBJS)
	$(LD) -shared -o $@ $^

romfs:
	$(ROMFSINST) -S /lib/libshared.so

clean:
	rm -f *.o libshared.so

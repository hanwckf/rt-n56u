
##################################################################
# Define Ralink-based boards
# Board descriptors placed in include/ralink_boards.h
##################################################################
# Board PID # Board Name       # PRODUCT # Note
##################################################################
# RT-N11P   # ASUS RT-N11P     # MT7620  #
# RT-N14U   # ASUS RT-N14U     # MT7620  #
# RT-AC51U  # ASUS RT-AC51U    # MT7620  #
# RT-AC52U  # ASUS RT-AC52U    # MT7620  #
# RT-N56U   # ASUS RT-N56U     # RT3883  #
# RT-N65U   # ASUS RT-N65U     # RT3883  #
# SWR1100   # Samsung SWR-1100 # RT3883  #
# BN750DB   # Belkin N750 DB   # RT3883  # Need fix Uboot PID
##################################################################

BOARD_HAS_5G_RADIO=1
BOARD_NUM_USB_PORTS=1

ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N11P")
CFLAGS += -DVENDOR_ASUS -DBOARD_N11P
BOARD_HAS_5G_RADIO=0
BOARD_NUM_USB_PORTS=0
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N14U")
CFLAGS += -DVENDOR_ASUS -DBOARD_N14U
BOARD_HAS_5G_RADIO=0
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-AC51U")
CFLAGS += -DVENDOR_ASUS -DBOARD_AC51U
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-AC52U")
CFLAGS += -DVENDOR_ASUS -DBOARD_AC52U
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N56U")
CFLAGS += -DVENDOR_ASUS -DBOARD_N56U
BOARD_NUM_USB_PORTS=2
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N65U")
CFLAGS += -DVENDOR_ASUS -DBOARD_N65U
BOARD_NUM_USB_PORTS=2
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"SWR1100")
CFLAGS += -DBOARD_SWR1100
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"BN750DB")
CFLAGS += -DBOARD_BN750DB
BOARD_NUM_USB_PORTS=2
endif

##################################################################

ifndef CONFIG_USB_SUPPORT
BOARD_NUM_USB_PORTS=0
endif

CFLAGS += -DBOARD_HAS_5G_RADIO=$(BOARD_HAS_5G_RADIO)
CFLAGS += -DBOARD_NUM_USB_PORTS=$(BOARD_NUM_USB_PORTS)

export BOARD_HAS_5G_RADIO
export BOARD_NUM_USB_PORTS


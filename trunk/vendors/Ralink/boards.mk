
##################################################################
# Define Ralink-based boards
# Board descriptors placed in user/shared/include/ralink_boards.h
##################################################################
# Board PID # Board Name       # PRODUCT # Note
##################################################################
# RT-N14U   # ASUS RT-N14U     # MT7620  #
# RT-N56U   # ASUS RT-N56U     # RT3883  #
# RT-N65U   # ASUS RT-N65U     # RT3883  #
# SWR1100   # Samsung SWR-1100 # RT3883  #
# BN750DB   # Belkin N750 DB   # RT3883  # Need fix Uboot PID
##################################################################

ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N14U")
CFLAGS += -DBOARD_N14U
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N56U")
CFLAGS += -DBOARD_N56U
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"RT-N65U")
CFLAGS += -DBOARD_N65U
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"SWR1100")
CFLAGS += -DBOARD_SWR1100
endif
ifeq ($(CONFIG_FIRMWARE_PRODUCT_ID),"BN750DB")
CFLAGS += -DBOARD_BN750DB
endif

##################################################################
# Define board RAM size from Linux kernel config
##################################################################

CFLAGS += -DBOARD_RAM_SIZE=$(CONFIG_RALINK_RAM_SIZE)

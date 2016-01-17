##################################################################
# Define Ralink/Mediatek boards
##################################################################

BOARD_NUM_USB_PORTS=0

include $(ROOTDIR)/user/shared/board.mk

##################################################################

BOARD_2G_IN_SOC=0
BOARD_5G_IN_SOC=0
BOARD_HAS_5G_RADIO=0

ifndef CONFIG_USB_SUPPORT
BOARD_NUM_USB_PORTS=0
endif

ifdef CONFIG_RT_FIRST_IF_RANGE_2GHZ
ifdef CONFIG_RT_FIRST_IF_RT2860
BOARD_2G_IN_SOC=1
endif
ifdef CONFIG_RT_FIRST_IF_MT7628
BOARD_2G_IN_SOC=1
endif
endif

ifdef CONFIG_RT_FIRST_IF_RANGE_5GHZ
BOARD_HAS_5G_RADIO=1
ifdef CONFIG_RT_FIRST_IF_RT2860
BOARD_5G_IN_SOC=1
endif
endif

ifdef CONFIG_RT_SECOND_IF_RANGE_5GHZ
BOARD_HAS_5G_RADIO=1
endif

CFLAGS += -DBOARD_2G_IN_SOC=$(BOARD_2G_IN_SOC)
CFLAGS += -DBOARD_5G_IN_SOC=$(BOARD_5G_IN_SOC)
CFLAGS += -DBOARD_HAS_5G_RADIO=$(BOARD_HAS_5G_RADIO)
CFLAGS += -DBOARD_NUM_USB_PORTS=$(BOARD_NUM_USB_PORTS)

export BOARD_2G_IN_SOC
export BOARD_5G_IN_SOC
export BOARD_HAS_5G_RADIO
export BOARD_NUM_USB_PORTS

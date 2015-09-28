#####################################################################
# Please executes 'make menuconfig' to config your own uboot
#####################################################################
-include $(TOPDIR)/.config

#################<< Test function option Configuration >>###################
#----------------------------------------
#If your GPIO14 connect to hardware reset circuit,
#you can turn on the definition to reset whole board
#if cpu_reset, system_reset or wdg_reset occured
#----------------------------------------
#BOARD_RESET_MODE = GPIO14_RESET_MODE

#----------------------------------------
#If using an 32MB flash on RT3052_MP2, you might try to turn this on.
#----------------------------------------
#ON_BOARD_32M_FLASH_COMPONENT = y

#----------------------------------------
# PCI pin config to GPIO mode
#----------------------------------------
#PCI_MODE = PCI_FUNC
#UARTF_MODE = UARTF_FUNC

######## RT2880 test function option configuration ##########################
RALINK_DDR_CONTROLLER_OPTIMIZATION = OFF
RALINK_CPU_AUTO_FREQUENCY = OFF
RALINK_SDR_PRECHARGE_POWER_DOWN = OFF		   # for mt7620, it is already enabled by default.
RALINK_DDR_SELF_REFRESH_POWER_SAVE_MODE = OFF	   # for mt7620, it is already enabled by default.
RALINK_SPI_UPGRADE_CHECK = ON
RALINK_NAND_UPGRADE_CHECK = OFF
ifeq ($(ON_BOARD_NAND_FLASH_COMPONENT),y)
ifeq ($(MT7621_ASIC_BOARD),y)
RALINK_RW_RF_REG_FUN = OFF
else
RALINK_RW_RF_REG_FUN = ON
endif
else
RALINK_RW_RF_REG_FUN = ON
endif
RALINK_USB = OFF
RALINK_EHCI = OFF
RALINK_OHCI = OFF
MTK_XHCI = OFF
RALINK_SSO_TEST_FUN = OFF
RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1 = OFF
RALINK_SPI_CS0_HIGH_ACTIVE = OFF
RALINK_SPI_CS1_HIGH_ACTIVE = OFF
MTK_MSDC = OFF
EMMC_8BIT = OFF

#Only for built-in 10/100/1000 Embedded Switch
RALINK_EPHY_TESTER = OFF

#Only for built-in 10/100 Embedded Switch
RALINK_SWITCH_DEBUG_FUN = OFF

###################################
# Optimized for Size flag
###################################
ifeq ($(ON_BOARD_NAND_FLASH_COMPONENT),y)
RALINK_UPGRADE_BY_SERIAL = OFF
else
RALINK_UPGRADE_BY_SERIAL = ON
endif
RALINK_CMDLINE = ON
RALINK_MDIO_ACCESS_FUN = ON
RALINK_EPHY_INIT = ON

ifeq ($(CONFIG_TINY_UBOOT), y)
RALINK_SPI_UPGRADE_CHECK = OFF
RALINK_UPGRADE_BY_SERIAL = OFF
ifeq ($(MT7620_ASIC_BOARD),y)
RALINK_RW_RF_REG_FUN = ON
else
RALINK_RW_RF_REG_FUN = OFF
endif
RALINK_CMDLINE = OFF
endif

##############################
# Decompression Algorithm
##############################
CONFIG_GZIP = OFF
CONFIG_BZIP2 = OFF
CONFIG_LZMA = ON
CONFIG_XZ = OFF

##########################################################################

# clean the slate ...
PLATFORM_RELFLAGS =
PLATFORM_CPPFLAGS =
PLATFORM_LDFLAGS =

#
# When cross-compiling on NetBSD, we have to define __PPC__ or else we
# will pick up a va_list declaration that is incompatible with the
# actual argument lists emitted by the compiler.
#
# [Tested on NetBSD/i386 1.5 + cross-powerpc-netbsd-1.3]

ifeq ($(ARCH),ppc)
ifeq ($(CROSS_COMPILE),powerpc-netbsd-)
PLATFORM_CPPFLAGS+= -D__PPC__
endif
ifeq ($(CROSS_COMPILE),powerpc-openbsd-)
PLATFORM_CPPFLAGS+= -D__PPC__
endif
endif

ifeq ($(ARCH),arm)
ifeq ($(CROSS_COMPILE),powerpc-netbsd-)
PLATFORM_CPPFLAGS+= -D__ARM__
endif
ifeq ($(CROSS_COMPILE),powerpc-openbsd-)
PLATFORM_CPPFLAGS+= -D__ARM__
endif
endif

ifdef	ARCH
sinclude $(TOPDIR)/$(ARCH)_config.mk	# include architecture dependend rules
endif
ifdef	CPU
sinclude $(TOPDIR)/cpu/$(CPU)/config.mk	# include  CPU	specific rules
endif
ifdef	SOC
sinclude $(TOPDIR)/cpu/$(CPU)/$(SOC)/config.mk	# include  SoC	specific rules
endif
ifdef	VENDOR
BOARDDIR = $(VENDOR)/$(BOARD)
else
BOARDDIR = $(BOARD)
endif
ifdef	BOARD
sinclude $(TOPDIR)/board/$(BOARDDIR)/config.mk	# include board specific rules
endif

#########################################################################

CONFIG_SHELL	:= $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
		    else if [ -x /bin/bash ]; then echo /bin/bash; \
		    else echo sh; fi ; fi)

ifeq ($(HOSTOS)-$(HOSTARCH),darwin-ppc)
HOSTCC		= cc
else
HOSTCC		= gcc
endif
HOSTCFLAGS	= -Wall -Wstrict-prototypes -g -fomit-frame-pointer
HOSTSTRIP	= strip

#########################################################################

#
# Include the make variables (CC, etc...)
#
AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC) -E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)RANLIB

RELFLAGS= $(PLATFORM_RELFLAGS)
DBGFLAGS= -gdwarf-2 -DDEBUG
OPTFLAGS= -Os
ifndef LDSCRIPT
#LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot.lds.debug
LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot.lds
endif
OBJCFLAGS += --gap-fill=0xff

gccincdir := $(shell $(CC) -print-file-name=include)


CPPFLAGS := $(DBGFLAGS) $(OPTFLAGS) $(RELFLAGS)		\
	-D__KERNEL__ -DTEXT_BASE=$(TEXT_BASE) 		\
	-I$(TOPDIR)/include				\
	-fno-builtin -ffreestanding -nostdinc -isystem	\
	$(gccincdir) -pipe $(PLATFORM_CPPFLAGS)
#	-DROUTER100					\

ifeq ($(MT7621_MP),y)
CPPFLAGS += -EL -mmt -mips32r2
else
CPPFLAGS += -march=4kc -mtune=4kc
endif

ifeq ($(UN_NECESSITY_U_BOOT_CMD_OPEN),ON)
CPPFLAGS += -DRT2880_U_BOOT_CMD_OPEN
endif

ifeq ($(RALINK_MDIO_ACCESS_FUN),ON)
CPPFLAGS += -DRALINK_MDIO_ACCESS_FUN
endif

ifeq ($(RALINK_SWITCH_DEBUG_FUN),ON)
CPPFLAGS += -DRALINK_SWITCH_DEBUG_FUN
endif

ifeq ($(RALINK_DUAL_CORE_FUN),y)
CPPFLAGS += -DRALINK_DUAL_CORE_FUN
endif

ifeq ($(RALINK_DUAL_VPE_FUN),y)
CPPFLAGS += -DRALINK_DUAL_VPE_FUN
endif

ifeq ($(RALINK_DDR_CONTROLLER_OPTIMIZATION),ON)
CPPFLAGS += -DRALINK_DDR_OPTIMIZATION
endif

ifeq ($(RALINK_CPU_AUTO_FREQUENCY),ON)
CPPFLAGS += -DRALINK_CPU_AUTOFREQUENCY
endif

ifeq ($(RALINK_SDR_PRECHARGE_POWER_DOWN),ON)
CPPFLAGS += -DRALINK_SDR_POWERSAVE
endif

ifeq ($(RALINK_DDR_SELF_REFRESH_POWER_SAVE_MODE),ON)
CPPFLAGS += -DRALINK_DDR_POWERSAVE
endif

# for MT7620, enable sdr/ddr1/ddr2 power saving by default.
ifeq ($(MT7620_ASIC_BOARD),y)
ifeq ($(ON_BOARD_SDR),y)
CPPFLAGS += -DRALINK_SDR_POWERSAVE
endif
ifeq ($(ON_BOARD_DDR1),y)
CPPFLAGS += -DRALINK_DDR_POWERSAVE
endif
ifeq ($(ON_BOARD_DDR2),y)
CPPFLAGS += -DRALINK_DDR_POWERSAVE
endif
endif

ifeq ($(MT7628_ASIC_BOARD),y)
ifeq ($(ON_BOARD_DDR1),y)
CPPFLAGS += -DRALINK_DDR_POWERSAVE -DCONFIG_DDR_CAL
endif
ifeq ($(ON_BOARD_DDR2),y)
CPPFLAGS += -DRALINK_DDR_POWERSAVE -DCONFIG_DDR_CAL
endif
endif

ifeq ($(RALINK_SPI_UPGRADE_CHECK),ON)
CPPFLAGS += -DRALINK_SPI_UPGRADE_CHECK
endif

ifeq ($(RALINK_NAND_UPGRADE_CHECK),ON)
CPPFLAGS += -DRALINK_NAND_UPGRADE_CHECK
endif

ifeq ($(RALINK_RW_RF_REG_FUN),ON)
CPPFLAGS += -DRALINK_RW_RF_REG_FUN
endif

ifeq ($(RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1),ON)
CPPFLAGS += -DRALINK_VITESSE_SWITCH_CONNECT_SPI_CS1
endif

ifeq ($(RALINK_SPI_CS0_HIGH_ACTIVE),ON)
CPPFLAGS += -DRALINK_SPI_CS0_HIGH_ACTIVE
endif

ifeq ($(RALINK_SPI_CS1_HIGH_ACTIVE),ON)
CPPFLAGS += -DRALINK_SPI_CS1_HIGH_ACTIVE
endif

ifeq ($(RALINK_EPHY_TESTER),ON)
CPPFLAGS += -DRALINK_EPHY_TESTER
endif

ifeq ($(RALINK_UPGRADE_BY_SERIAL),ON)
CPPFLAGS += -DRALINK_UPGRADE_BY_SERIAL
endif

ifeq ($(RALINK_CMDLINE),ON)
CPPFLAGS += -DRALINK_CMDLINE
endif

ifeq ($(RALINK_EPHY_INIT),ON)
CPPFLAGS += -DRALINK_EPHY_INIT
endif

ifeq ($(CONFIG_GZIP),ON)
CPPFLAGS += -DCONFIG_GZIP
endif

ifeq ($(CONFIG_BZIP2),ON)
CPPFLAGS += -DCONFIG_BZIP2
endif

ifeq ($(CONFIG_LZMA),ON)
CPPFLAGS += -DCONFIG_LZMA
endif

ifeq ($(CONFIG_XZ),ON)
CPPFLAGS += -DCONFIG_XZ
endif

ifeq ($(RALINK_OHCI),ON)
CPPFLAGS += -DRALINK_USB -DRALINK_OHCI
RALINK_USB = ON
endif

ifeq ($(RALINK_EHCI),ON)
CPPFLAGS += -DRALINK_USB -DRALINK_EHCI
RALINK_USB = ON
endif

ifeq ($(MTK_XHCI),ON)
CPPFLAGS += -DMTK_USB -DCONFIG_RALINK_MT7621 -DCONFIG_USB_XHCI
MTK_USB = ON
# save size for XHCI code
RALINK_SPI_UPGRADE_CHECK = OFF
RALINK_UPGRADE_BY_SERIAL = OFF
RALINK_RW_RF_REG_FUN = OFF
endif

ifeq ($(RALINK_SSO_TEST_FUN),ON)
CPPFLAGS += -DRALINK_SSO_TEST_FUN
endif

ifeq ($(P5_MAC_TO_NONE_MODE),y)
CPPFLAGS += -DP5_MAC_TO_NONE_MODE
endif

ifeq ($(P5_MAC_TO_PHY_MODE),y)
CPPFLAGS += -DP5_MAC_TO_PHY_MODE
endif

ifeq ($(P5_RGMII_TO_MAC_MODE),y)
CPPFLAGS += -DP5_RGMII_TO_MAC_MODE
endif

ifeq ($(P5_MII_TO_MAC_MODE),y)
CPPFLAGS += -DP5_MII_TO_MAC_MODE
endif

ifeq ($(P5_RMII_TO_MAC_MODE),y)
CPPFLAGS += -DP5_RMII_TO_MAC_MODE
endif

ifeq ($(P4_MAC_TO_NONE_MODE),y)
CPPFLAGS += -DP4_MAC_TO_NONE_MODE
endif

ifeq ($(P4_MAC_TO_PHY_MODE),y)
CPPFLAGS += -DP4_MAC_TO_PHY_MODE
endif

ifeq ($(P4_RGMII_TO_MAC_MODE),y)
CPPFLAGS += -DP4_RGMII_TO_MAC_MODE
endif

ifeq ($(P4_MII_TO_MAC_MODE),y)
CPPFLAGS += -DP4_MII_TO_MAC_MODE
endif

ifeq ($(P4_RMII_TO_MAC_MODE),y)
CPPFLAGS += -DP4_RMII_TO_MAC_MODE
endif

ifeq ($(MTK_MSDC),ON)
CPPFLAGS += -DMTK_MSDC
endif

ifeq ($(EMMC_8BIT),ON)
CPPFLAGS += -DEMMC_8BIT
endif

ifeq ($(ASIC_BOARD),y)
CPPFLAGS += -DASIC_BOARD
endif

ifeq ($(FPGA_BOARD),y)
CPPFLAGS += -DFPGA_BOARD
endif

ifeq ($(RT2880_FPGA_BOARD),y)
CPPFLAGS += -DRT2880_FPGA_BOARD
endif

ifeq ($(RT2880_ASIC_BOARD),y)
CPPFLAGS += -DRT2880_ASIC_BOARD
endif

ifeq ($(RT2883_FPGA_BOARD),y)  
CPPFLAGS += -DRT2883_FPGA_BOARD
endif

ifeq ($(RT2883_ASIC_BOARD),y)  
CPPFLAGS += -DRT2883_ASIC_BOARD
endif

ifeq ($(RT3350_ASIC_BOARD),y)
CPPFLAGS += -DRT3350_ASIC_BOARD
endif

ifeq ($(RT5350_FPGA_BOARD),y)
CPPFLAGS += -DRT5350_FPGA_BOARD
endif

ifeq ($(RT5350_ASIC_BOARD),y)
CPPFLAGS += -DRT5350_ASIC_BOARD
endif

ifeq ($(RT3352_FPGA_BOARD),y)
CPPFLAGS += -DRT3352_FPGA_BOARD
endif

ifeq ($(RT3352_ASIC_BOARD),y)
CPPFLAGS += -DRT3352_ASIC_BOARD
endif

ifeq ($(RT6855_FPGA_BOARD),y)
CPPFLAGS += -DRT6855_FPGA_BOARD
endif

ifeq ($(RT6855_ASIC_BOARD),y)
CPPFLAGS += -DRT6855_ASIC_BOARD
endif

ifeq ($(MT7620_FPGA_BOARD),y)
CPPFLAGS += -DMT7620_FPGA_BOARD
endif

ifeq ($(MT7620_ASIC_BOARD),y)
CPPFLAGS += -DMT7620_ASIC_BOARD
endif

ifeq ($(MT7628_FPGA_BOARD),y)
CPPFLAGS += -DMT7628_FPGA_BOARD
endif

ifeq ($(MT7628_ASIC_BOARD),y)
CPPFLAGS += -DMT7628_ASIC_BOARD
endif

ifeq ($(MT7621_FPGA_BOARD),y)
CPPFLAGS += -DMT7621_FPGA_BOARD
endif

ifeq ($(MT7621_ASIC_BOARD),y)
CPPFLAGS += -DMT7621_ASIC_BOARD
endif


ifeq ($(RT3052_FPGA_BOARD),y)
CPPFLAGS += -DRT3052_FPGA_BOARD
endif

ifeq ($(RT3052_ASIC_BOARD),y)
CPPFLAGS += -DRT3052_ASIC_BOARD
endif

ifeq ($(RT3883_FPGA_BOARD),y)  
CPPFLAGS += -DRT3883_FPGA_BOARD
endif

ifeq ($(RT3883_ASIC_BOARD),y)  
CPPFLAGS += -DRT3883_ASIC_BOARD
endif

ifeq ($(RT6855A_FPGA_BOARD),y)  
CPPFLAGS += -DRT6855A_FPGA_BOARD
endif

ifeq ($(RT6855A_ASIC_BOARD),y)  
CPPFLAGS += -DRT6855A_ASIC_BOARD
endif

ifeq ($(RT2880_MP),y)
CPPFLAGS += -DRT2880_MP
endif

ifeq ($(RT2883_MP),y)
CPPFLAGS += -DRT2883_MP
endif

ifeq ($(RT3052_MP2),y)
CPPFLAGS += -DRT3052_MP2
endif

ifeq ($(RT3352_MP),y)
CPPFLAGS += -DRT3352_MP
endif

ifeq ($(RT6855_MP),y)
CPPFLAGS += -DRT6855_MP
endif

ifeq ($(MT7620_MP),y)
CPPFLAGS += -DMT7620_MP
endif

ifeq ($(MT7628_MP),y)
CPPFLAGS += -DMT7628_MP
endif

ifeq ($(MT7621_MP),y)
CPPFLAGS += -DMT7621_MP
endif

ifeq ($(RT3883_MP),y)
CPPFLAGS += -DRT3883_MP
endif

ifeq ($(RT5350_MP),y)
CPPFLAGS += -DRT5350_MP
endif

ifeq ($(RT6855A_MP),y)
CPPFLAGS += -DRT6855A_MP
endif

ifeq ($(MAC_TO_VITESSE_MODE),y)
CPPFLAGS += -DMAC_TO_VITESSE_MODE
endif

ifeq ($(MAC_TO_RTL8367_MODE),y)
CPPFLAGS += -DMAC_TO_RTL8367_MODE
endif

ifeq ($(MAC_TO_MT7530_MODE),y)
CPPFLAGS += -DMAC_TO_MT7530_MODE
endif

ifeq ($(MAC_TO_GIGAPHY_MODE),y)
CPPFLAGS += -DMAC_TO_GIGAPHY_MODE
endif

ifdef MAC_TO_GIGAPHY_MODE_ADDR
CPPFLAGS += -DMAC_TO_GIGAPHY_MODE_ADDR=$(MAC_TO_GIGAPHY_MODE_ADDR)
endif 

ifdef MAC_TO_GIGAPHY_MODE_ADDR2
CPPFLAGS += -DMAC_TO_GIGAPHY_MODE_ADDR2=$(MAC_TO_GIGAPHY_MODE_ADDR2)
endif 

ifeq ($(MAC_TO_100SW_MODE),y)
CPPFLAGS += -DMAC_TO_100SW_MODE
endif

ifeq ($(MAC_TO_100PHY_MODE),y)
CPPFLAGS += -DMAC_TO_100PHY_MODE
endif

ifeq ($(GPIOx_RESET_MODE),y)
CPPFLAGS += -DGPIOx_RESET_MODE
endif

ifeq ($(BOARD_RESET_MODE),GPIO14_RESET_MODE)
CPPFLAGS += -DGPIO14_RESET_MODE
endif

ifeq ($(RT3883_USE_GE2),y)
CPPFLAGS += -DRT3883_USE_GE2
endif

ifeq ($(MT7621_USE_GE2),y)
CPPFLAGS += -DMT7621_USE_GE2
endif

ifeq ($(MT7621_USE_GE1),y)
CPPFLAGS += -DMT7621_USE_GE1
endif

ifeq ($(GE_RGMII_INTERNAL_P0_AN),y)
CPPFLAGS += -DGE_RGMII_INTERNAL_P0_AN
endif

ifeq ($(GE_RGMII_INTERNAL_P4_AN),y)
CPPFLAGS += -DGE_RGMII_INTERNAL_P4_AN
endif

ifeq ($(BYPASS_MTK_DDR_CAL),y)
CPPFLAGS += -DBYPASS_MTK_DDR_CAL
endif

ifeq ($(GE_MII_FORCE_100),y)
CPPFLAGS += -DGE_MII_FORCE_100
endif

ifeq ($(GE_RVMII_FORCE_100),y)
CPPFLAGS += -DGE_RVMII_FORCE_100
endif

ifeq ($(GE_MII_AN),y)
CPPFLAGS += -DGE_MII_AN
endif

ifeq ($(GE_RGMII_FORCE_1000),y)
CPPFLAGS += -DGE_RGMII_FORCE_1000
endif

ifeq ($(GE_RGMII_AN),y)
CPPFLAGS += -DGE_RGMII_AN
endif

ifeq ($(PCI_MODE),PCI_FUNC)
CPPFLAGS += -DPCI_AT_GPIO_FUNC
endif

ifeq ($(UARTF_MODE),UARTF_FUNC)
CPPFLAGS += -DUARTF_AT_GPIO_FUNC
endif

ifeq ($(RALINK_DEMO_BOARD_PVLAN),y)
CPPFLAGS += -DRALINK_DEMO_BOARD_PVLAN
endif
ifeq ($(RALINK_EV_BOARD_PVLAN),y)
CPPFLAGS += -DRALINK_EV_BOARD_PVLAN
endif

ifeq ($(DUAL_IMAGE_SUPPORT),y)
CPPFLAGS += -DDUAL_IMAGE_SUPPORT
endif

ifeq ($(PDMA_OLD),y)
CPPFLAGS += -DPDMA_OLD
endif

ifeq ($(PDMA_NEW),y)
CPPFLAGS += -DPDMA_NEW
endif

ifeq ($(RX_SCATTER_GATTER_DMA),y)
CPPFLAGS += -DRX_SCATTER_GATTER_DMA
endif

ifeq ($(UBOOT_RAM),y)
CPPFLAGS += -DUBOOT_RAM
endif

ifeq ($(UBOOT_ROM),y)
CPPFLAGS += -DUBOOT_ROM
endif

ifeq ($(SMALL_UBOOT_PARTITION),y)
CPPFLAGS += -DSMALL_UBOOT_PARTITION
endif

ifeq ($(ON_BOARD_SDR),y)
CPPFLAGS += -DON_BOARD_SDR
endif
ifeq ($(ON_BOARD_DDR1),y)
CPPFLAGS += -DON_BOARD_DDR1
endif
ifeq ($(ON_BOARD_DDR2),y)
CPPFLAGS += -DON_BOARD_DDR2
endif
ifeq ($(ON_BOARD_DDR3),y)
CPPFLAGS += -DON_BOARD_DDR3
endif

ifeq ($(ON_BOARD_DDR_WIDTH_16),y)
CPPFLAGS += -DON_BOARD_DDR_WIDTH_16
endif
ifeq ($(ON_BOARD_DDR_WIDTH_8),y)
CPPFLAGS += -DON_BOARD_DDR_WIDTH_8
endif

ifeq ($(ON_BOARD_16BIT_DRAM_BUS),y)
CPPFLAGS += -DON_BOARD_16BIT_DRAM_BUS
else
ifeq ($(ON_BOARD_32BIT_DRAM_BUS),y)
CPPFLAGS += -DON_BOARD_32BIT_DRAM_BUS
endif
endif

ifeq ($(ON_BOARD_16M_DRAM_COMPONENT),y)
CPPFLAGS += -DON_BOARD_16M_DRAM_COMPONENT
else
ifeq ($(ON_BOARD_64M_DRAM_COMPONENT),y)
CPPFLAGS += -DON_BOARD_64M_DRAM_COMPONENT
else
ifeq ($(ON_BOARD_128M_DRAM_COMPONENT),y)
CPPFLAGS += -DON_BOARD_128M_DRAM_COMPONENT
else
ifeq ($(ON_BOARD_256M_DRAM_COMPONENT),y)
CPPFLAGS += -DON_BOARD_256M_DRAM_COMPONENT
else
ifeq ($(ON_BOARD_512M_DRAM_COMPONENT),y)
CPPFLAGS += -DON_BOARD_512M_DRAM_COMPONENT
else
ifeq ($(ON_BOARD_1024M_DRAM_COMPONENT),y)
CPPFLAGS += -DON_BOARD_1024M_DRAM_COMPONENT
else
ifeq ($(ON_BOARD_2048M_DRAM_COMPONENT),y)
CPPFLAGS += -DON_BOARD_2048M_DRAM_COMPONENT
else
ifeq ($(ON_BOARD_4096M_DRAM_COMPONENT),y)
CPPFLAGS += -DON_BOARD_4096M_DRAM_COMPONENT
endif
endif
endif
endif
endif
endif
endif
endif

ifeq ($(CPU_PLL_PARAMETERS),y)
CPPFLAGS += -DCPU_PLL_PARAMETERS
endif

ifeq ($(ON_BOARD_2M_FLASH_COMPONENT),y)
CPPFLAGS += -DON_BOARD_2M_FLASH_COMPONENT
endif

ifeq ($(ON_BOARD_4M_FLASH_COMPONENT),y)
CPPFLAGS += -DON_BOARD_4M_FLASH_COMPONENT
endif

ifeq ($(ON_BOARD_8M_FLASH_COMPONENT),y)
CPPFLAGS += -DON_BOARD_8M_FLASH_COMPONENT
endif

ifeq ($(ON_BOARD_16M_FLASH_COMPONENT),y)
CPPFLAGS += -DON_BOARD_16M_FLASH_COMPONENT
endif

ifeq ($(ON_BOARD_32M_FLASH_COMPONENT),y)
CPPFLAGS += -DON_BOARD_32M_FLASH_COMPONENT
endif

ifeq ($(ON_BOARD_NAND_FLASH_COMPONENT),y)
CFG_ENV_IS := IN_NAND
ifeq ($(MT7621_MP), y)
CPPFLAGS += -DMTK_NAND
endif	
else
ifeq ($(ON_BOARD_SPI_FLASH_COMPONENT),y)
CFG_ENV_IS := IN_SPI
else
CFG_ENV_IS := IN_FLASH
endif
endif
CPPFLAGS += -DCFG_ENV_IS_$(CFG_ENV_IS)

ifdef BUILD_TAG
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes \
	-DBUILD_TAG='"$(BUILD_TAG)"'
else
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes
endif

# avoid trigraph warnings while parsing pci.h (produced by NIOS gcc-2.9)
# this option have to be placed behind -Wall -- that's why it is here
ifeq ($(ARCH),nios)
ifeq ($(findstring 2.9,$(shell $(CC) --version)),2.9)
CFLAGS := $(CPPFLAGS) -Wall -Wno-trigraphs
endif
endif

CFLAGS += -Wno-unused -Wno-unused-variable -Wno-unused-but-set-variable

AFLAGS_DEBUG := -Wa,-gstabs
AFLAGS := $(AFLAGS_DEBUG) -D__ASSEMBLY__ $(CPPFLAGS)
ifeq ($(CONFIG_TINY_UBOOT), y)
CONFIG_CROSS_COMPILER_PATH = /opt/buildroot-gcc463/usr/bin
CFLAGS += -fpic -G0 -mips16 -fomit-frame-pointer -mno-long-calls -DCONFIG_TINY_UBOOT -DCONFIG_MIPS16
AFLAGS += -fpic -mabicalls
endif

ifeq ($(MT7621_MP),y)
CFLAGS += -G0 -fomit-frame-pointer -Wno-pointer-sign
endif

#ifeq ($(MT7621_MP),y)
#CFLAGS += -fpic
#AFLAGS += -fno-pic
#endif

LDFLAGS += -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS)

# Location of a usable BFD library, where we define "usable" as
# "built for ${HOST}, supports ${TARGET}".  Sensible values are
# - When cross-compiling: the root of the cross-environment
# - Linux/ppc (native): /usr
# - NetBSD/ppc (native): you lose ... (must extract these from the
#   binutils build directory, plus the native and U-Boot include
#   files don't like each other)
#
# So far, this is used only by tools/gdb/Makefile.

ifeq ($(HOSTOS)-$(HOSTARCH),darwin-ppc)
BFD_ROOT_DIR =		/usr/local/tools
else
ifeq ($(HOSTARCH),$(ARCH))
# native
BFD_ROOT_DIR =		/usr
else
#BFD_ROOT_DIR =		/LinuxPPC/CDK		# Linux/i386
#BFD_ROOT_DIR =		/usr/pkg/cross		# NetBSD/i386
BFD_ROOT_DIR =		/opt/powerpc
endif
endif

#########################################################################

export	CONFIG_SHELL HPATH HOSTCC HOSTCFLAGS CROSS_COMPILE \
	AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP \
	MAKE
export	TEXT_BASE PLATFORM_CPPFLAGS PLATFORM_RELFLAGS CPPFLAGS CFLAGS AFLAGS
#########################################################################

%.s:	%.S
	$(CPP) $(AFLAGS) -o $@ $(CURDIR)/$<
%.o:	%.S
	$(CC) $(AFLAGS) -c -o $@ $(CURDIR)/$<
%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

#########################################################################

#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

# CFLAGS += -DET_DEBUG -DDEBUG

# for RTL8367M
#CFLAGS += -DMDC_MDIO_OPERATION
CFLAGS += -D_LITTLE_ENDIAN

LIB	= libdrivers.a

OBJS	= rt2880_eth.o i2c_drv.o mii_mgr.o

OBJS += ralink_smi.o rtl8367.o

OBJS += rtl8370/rtk_api.o
OBJS += rtl8370/rtl8370_asicdrv.o
OBJS += rtl8370/rtl8370_asicdrv_led.o
OBJS += rtl8370/rtl8370_asicdrv_lut.o
OBJS += rtl8370/rtl8370_asicdrv_phy.o
OBJS += rtl8370/rtl8370_asicdrv_port.o
OBJS += rtl8370/rtl8370_asicdrv_portIsolation.o

ifeq ($(CFG_ENV_IS), IN_FLASH)
#OBJS	+= spi_drv.o
endif

ifeq ($(CFG_ENV_IS), IN_SPI)
OBJS	+= spi_flash.o 
 ifeq ($(RT3883_MP), y)
# OBJS	+= spi_drv.o 
 endif
 ifeq ($(RT3352_MP), y)
# OBJS	+= spi_drv.o 
 endif
endif

ifeq ($(CFG_ENV_IS), IN_NAND)
OBJS	+= nand_flash.o 
 ifeq ($(RT3883_MP), y)
 OBJS	+= spi_drv.o 
 endif
endif

ifeq ($(RALINK_OHCI), ON)
OBJS	+= ohci-hcd.o
endif

ifeq ($(RALINK_EHCI), ON)
OBJS	+= ehci-hcd.o ehci-ra.o
endif

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) crv $@ $(OBJS)

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################

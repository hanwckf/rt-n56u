/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <rt_mmap.h>
#include <configs/rt2880.h>
#include "ralink_spi.h"
#include "vtss.h"

#if (CONFIG_COMMANDS & CFG_CMD_SPI) 
#define DBG

#define	RT2880_SPI_DUMP_STR		"dump"	/* Dump Content Command Prompt    */
#define	RT2880_SPI_READ_STR		"read"	/* SPI read operation */
#define	RT2880_SPI_WRITE_STR		"write"	/* SPI read operation */

#define RT2880_SPI_DUMP        2
#define RT2880_SPI_READ        3
#define RT2880_SPI_WRITE       5
#define RT2880_SPI_RD_VTSS     7
#define RT2880_SPI_WR_VTSS     9


/*---------------------------------------------------------------------*/
/* Symbol & Macro Definitions                                          */
/*---------------------------------------------------------------------*/
#define RT2880_REG(x)		(*((volatile u32 *)(x)))

#define IS_BUSY		(RT2880_REG(RT2880_SPISTAT_REG) & 0x01)

#define spi_busy_loop 3000
#define max_ee_busy_loop 500


/*
 * ATMEL AT25XXXX Serial EEPROM 
 * access type
 */

/* Instruction codes */
#define WREN_CMD	0x06
#define WRDI_CMD	0x04
#define RDSR_CMD	0x05
#define WRSR_CMD	0x01
#define READ_CMD	0x03
#define WRITE_CMD	0x02

/* STATUS REGISTER BIT */
#define RDY 0	/*  Busy Indicator Bit */
#define WEN 1	/*  Write Enable Bit   */
#define BP0 2	/* Block Write Protect Bit */
#define BP1 3	/* Block Write Protect Bit */
#define WPEN 7	/* Software Write Protect Enable Bit */


#define ENABLE	1
#define DISABLE	0


/*---------------------------------------------------------------------*/
/* Prototypes of External Functions                                    */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/* Prototypes of Functions Used                                        */
/*---------------------------------------------------------------------*/
static inline void spi_chip_select(u8 enable);
static void spi_write(u8 data);								
static u8 spi_read(void);

static void spi_master_init(void);
static u8 spi_eeprom_read(u16 address, u16 nbytes, u8 *dest);
static u8 spi_eeprom_write(u16 address, u16 nbytes, u8 *src);

int rt2880_spi_toolkit(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/*---------------------------------------------------------------------*/
/* External Variable Definitions                                       */
/*---------------------------------------------------------------------*/




/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_chip_select                                         */
/*    INPUTS: ENABLE or DISABLE                                         */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): Pull on or Pull down #CS                                  */
/*                                                                      */
/*----------------------------------------------------------------------*/

static inline void spi_chip_select(u8 enable)
{
	int i;

	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			/* low active */
			if (enable) {		
				RT2880_REG(RT2880_SPICTL_REG) =	SPICTL_SPIENA_LOW;
			} else  {
				RT2880_REG(RT2880_SPICTL_REG) = SPICTL_SPIENA_HIGH;
			}		
			break;
		}
	}

#ifdef DBG
	if (i == spi_busy_loop) {
		printf("warning : spi_transfer (spi_chip_select) busy !\n");
	}
#endif
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_master_init                                         */
/*    INPUTS: None                                                      */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): Initialize SPI block to desired state                     */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void spi_master_init(void)
{
	/* try to reset again from Reset Control Register */
	u32 val = RT2880_REG(RT2880_RSTCTRL_REG);
	val |= RSTCTRL_SPI_RESET;
	RT2880_REG(RT2880_RSTCTRL_REG) = val;

	val = val & ~(RSTCTRL_SPI_RESET);
	RT2880_REG(RT2880_RSTCTRL_REG) = val;
	udelay(500);

#if defined(RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)
	/* config ARB and set the low or high active correctly according to the device */
	RT2880_REG(RT2880_SPI_ARB_REG) = SPIARB_ARB_EN | (SPIARB_SPI1_ACTIVE_MODE <<1) | SPIARB_SPI0_ACTIVE_MODE;
        RT2880_REG(RT2880_SPI0_CTL_REG) = (~SPIARB_SPI0_ACTIVE_MODE)&0x1;     //disable first
        RT2880_REG(RT2880_SPI1_CTL_REG) = (~SPIARB_SPI1_ACTIVE_MODE)&0x1;     //disable first
#endif
	RT2880_REG(RT2880_SPICFG_REG) = SPICFG_MSBFIRST | 
									SPICFG_RXCLKEDGE_FALLING |
									SPICFG_TXCLKEDGE_FALLING |
									SPICFG_SPICLK_DIV8;
	spi_chip_select(DISABLE);

#ifdef DBG
/*        printf("SPICFG = %08x\n", RT2880_REG(RT2880_SPICFG_REG));*/
/*        printf("is busy %d\n", IS_BUSY);*/
	if (IS_BUSY) printf("spi_master_init: is busy\n");
#endif		 	
}



/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_write                                               */
/*    INPUTS: 8-bit data                                                */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): transfer 8-bit data to SPI                                */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void spi_write(u8 data)
{
	int i;

	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			RT2880_REG(RT2880_SPIDATA_REG) = data;
			/* start write transfer */
			RT2880_REG(RT2880_SPICTL_REG) = SPICTL_HIZSDO |  SPICTL_STARTWR | 
											SPICTL_SPIENA_LOW;
			break;
		}
	}

#ifdef DBG
	if (i == spi_busy_loop) {
		printf("warning : spi_transfer (write %02x) busy !\n", data);
	}
#endif
}

//write32 MSB first
static void spi_write32(u32 data)
{
	u8 d0, d1, d2, d3;

	d0 = (u8)((data >> 24) & 0xff);
	d1 = (u8)((data >> 16) & 0xff);
	d2 = (u8)((data >> 8) & 0xff);
	d3 = (u8)(data & 0xff);

	spi_write(d0);
	spi_write(d1);
	spi_write(d2);
	spi_write(d3);
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_read                                                */
/*    INPUTS: None                                                      */
/*   RETURNS: 8-bit data                                                */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): get 8-bit data from SPI                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/
static u8 spi_read(void) 
{
	int i;

	/* 
	 * poll busy bit until it is 0 
	 * then start read transfer
	 */
	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			RT2880_REG(RT2880_SPIDATA_REG) = 0;
			/* start read transfer */
			RT2880_REG(RT2880_SPICTL_REG) = SPICTL_HIZSDO | SPICTL_STARTRD |
											SPICTL_SPIENA_LOW;
			break;
		}
	}

	/* 
	 * poll busy bit until it is 0 
	 * then get data 
	 */
	for (i=0; i<spi_busy_loop; i++) {
		if (!IS_BUSY) {
			break;
		}
	}
	
#ifdef DBG		
	if (i == spi_busy_loop) {
		printf("warning : spi_transfer busy !\n");
	} 
#endif

	return ((u8)RT2880_REG(RT2880_SPIDATA_REG));
}

//read32 MSB first
static u32 spi_read32(void)
{
	u8 d0, d1, d2, d3;
	u32 ret;

	d0 = spi_read();
	d1 = spi_read();
	d2 = spi_read();
	d3 = spi_read();
	ret = (d0 << 24) | (d1 << 16) | (d2 << 8) | d3;

	return ret;
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_get_status_reg                                   */
/*    INPUTS: pointer to status                                         */
/*   RETURNS: None                                                      */
/*   OUTPUTS: status                                                    */
/*   NOTE(S): get the status of eeprom (AT25xxxx)                       */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void eeprom_get_status_reg(u8 *status) 
{
	spi_chip_select(ENABLE);
	spi_write(RDSR_CMD);
	*status = spi_read();		
	spi_chip_select(DISABLE);
}


/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_eeprom_read                                         */
/*    INPUTS: address - start address to be read                        */ 
/*            nbytes  - number of bytes to be read                      */
/*            dest    - pointer to read buffer                          */
/*   RETURNS: 0 - successful                                            */
/*            or eeprom status                                          */
/*   OUTPUTS: read buffer                                               */
/*   NOTE(S): If the eeprom is busy , the function returns with status  */
/*            register of eeprom                                        */
/*----------------------------------------------------------------------*/
static u8 spi_eeprom_read(u16 address, u16 nbytes, u8 *dest)
{
	u8	status;
	u16	cnt = 0;
	int i = 0;

	do {
		i++;
		eeprom_get_status_reg(&status);		
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

	if (i == max_ee_busy_loop)
		return (status);

	/* eeprom ready */
	if (!(status & (1<<RDY))) {	
		spi_chip_select(ENABLE);
		/* read op */
		spi_write(READ_CMD);
		spi_write((u8)(address >> 8));		/* MSB byte First */
		spi_write((u8)(address & 0x00FF));	/* LSB byte */

		while (cnt < nbytes) {
			*(dest++) = spi_read();
			cnt++;
		}
		status = 0;
		/* deassert cs */
		spi_chip_select(DISABLE);
	} 
	return (status);	
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	eeprom_write_enable                                     */
/*    INPUTS: None                                                      */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): always perform write enable  before any write operation   */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void eeprom_write_enable(void)
{
	u8	status;
	int i = 0;

	do {
		i++;
		eeprom_get_status_reg(&status);		
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

	if (i == max_ee_busy_loop)
		return;

	/* eeprom ready */
	if (!(status & (1<<RDY))) 
	{	
		spi_chip_select(ENABLE);
		/* always write enable  before any write operation */
		spi_write(WREN_CMD);

		spi_chip_select(DISABLE);
		
		/* wait for write enable */
		do {
			eeprom_get_status_reg(&status);
		} while((status & (1<<RDY)) || !(status & (1<<WEN)));

	}

}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	spi_eeprom_write                                        */
/*    INPUTS: address - the first byte address to be written            */
/*            nbytes  - the number of bytes to be written               */
/*            src     - the pointer to source buffer                    */     
/*   RETURNS: 0  - successful                                           */
/*            or eeprom buy status                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): The different eeprom has various write page size          */
/*            The function don't care write page size so the caller     */
/*            must check the page size of eeprom                        */
/*                                                                      */
/*----------------------------------------------------------------------*/
u8 spi_eeprom_write(u16 address, u16 nbytes, u8 *src)
{
	u8	status;
	u16	cnt = 0;
	int i = 0;

	do {
		i++;
		eeprom_get_status_reg(&status);		
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

	if (i == max_ee_busy_loop)
		goto done;


	/* eeprom ready */
	if (!(status & (1<<RDY))) {			
		/* always write enable  before any write operation */		
		eeprom_write_enable();

		spi_chip_select(ENABLE);
		spi_write(WRITE_CMD);
		spi_write((u8)(address >> 8));		/* MSB byte First */
		spi_write((u8)(address & 0x00FF));	/* LSB byte */

		while (cnt < nbytes) {
			spi_write(src[cnt]);
			cnt++;
		}
		status = 0;
		/* last byte sent then pull #cs high  */
		spi_chip_select(DISABLE);
	} 

	i = 0;
	do {
		i++;
		eeprom_get_status_reg(&status);		
	}
	while((status & (1<<RDY)) && (i < max_ee_busy_loop));

done:
	return (status);
}

void spi_eeprom_dump(void)
{
	printf("spi_eeprom_dump()... called!\n");
}

void spi_eeprom_cmd(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	int chk_match, size;
	ulong addr, value;
	u16 address;

	/* We use the last specified parameters, unless new ones are entered */
	switch (argc) {
		case RT2880_SPI_DUMP:
        		chk_match = strcmp(argv[1], RT2880_SPI_DUMP_STR);
			if ( chk_match != 0) {
				printf("Usage:\n%s\n", cmdtp->usage);
				return;
			}
			spi_eeprom_dump();
        		break;
		case RT2880_SPI_READ:
        		chk_match = strcmp(argv[1], RT2880_SPI_READ_STR);
        		if ( chk_match != 0) {
				printf("Usage:\n%s\n", cmdtp->usage);
                		return;
        		}
        		addr = simple_strtoul(argv[2], NULL, 16);
			address = addr;
			spi_master_init();
			spi_eeprom_read(addr, 4, (u8*)&value);
        		printf("0x%04x : 0x%04x\n", address, value);
        		break;
		case RT2880_SPI_WRITE:
			chk_match = strcmp(argv[1], RT2880_SPI_WRITE_STR);
			if ( chk_match != 0) {
				printf("Usage:\n%s\n", cmdtp->usage);
				return;
			}
			size = simple_strtoul(argv[2], NULL, 16);
			addr = simple_strtoul(argv[3], NULL, 16);
			value = simple_strtoul(argv[4], NULL, 16);
			spi_master_init();
			address = addr;
			spi_eeprom_write(address, size, (u8*)&value);
			printf("0x%08x: 0x%08x in %d bytes\n", address, value, size);
			break;
		default:
			printf("Usage:\n%s\n use \"help spicmd\" to get more detail!\n", cmdtp->usage);
	}
}


void spi_vtss_read(u8 blk, u8 subblk, u8 addr, u32 *value)
{
	u8 cmd;

	spi_master_init();
	cmd = (u8)((blk << 5) | subblk);
	spi_write(cmd);
	spi_write(addr);
	spi_read(); //dummy byte
	spi_read(); //dummy byte
	*value = spi_read32();
	//printf("rd %x:%x:%x = %x\n", blk, subblk, addr, *value);
	udelay(100);
}


void spi_vtss_write(u8 blk, u8 subblk, u8 addr, u32 value)
{
	u8 cmd;

	spi_master_init();
	cmd = (u8)((blk << 5) | subblk | 0x10);
	spi_write(cmd);
	spi_write(addr);
	spi_write32(value);
	//printf("wr %x:%x:%x = %x\n", blk, subblk, addr, value);
	udelay(10);
}

void vtss_init(void)
{
	int i, len, tmp;


 	//HT_WR(SYSTEM, 0, ICPU_CTRL, (1<<7) | (1<<3) | (1<<2) | (0<<0));
        //read it out to be sure the reset was done.
        while (1) {
                spi_vtss_write(7, 0, 0x10, (1<<7) | (1<<3) | (1<<2) | (0<<0));
                spi_vtss_read(7, 0, 0x10, &tmp);
                if (tmp & ((1<<7) | (1<<3) | (1<<2) | (0<<0)))
                        break;
                udelay(1000);
        }

        //HT_WR(SYSTEM, 0, ICPU_ADDR, 0); //clear SP_SELECT and ADDR
        spi_vtss_write(7, 0, 0x11, 0);

	len = sizeof(lutonu_novlan);
	for (i = 0; i < len; i++) {
		//HT_WR(SYSTEM, 0, ICPU_DATA, lutonu_novlan[i]);
		spi_vtss_write(7, 0, 0x12, lutonu_novlan[i]);
	}

	//HT_WR(SYSTEM, 0, GLORESET, (1<<0)); //MASTER_RESET
	spi_vtss_write(7, 0, 0x14, (1<<0));
	udelay(125000);

	//HT_WR(SYSTEM, 0, ICPU_CTRL, (1<<8) | (1<<3) | (1<<1) | (1<<0));
	spi_vtss_write(7, 0, 0x10, (1<<8) | (1<<3) | (1<<1) | (1<<0));
	printf(" Vitesse uploading binary codes (%d bytes) done.\n", len);
#if defined(RALINK_VITESSE_SWITCH_CONNECT_SPI_CS1)
        /* config ARB and set the low or high active correctly according to the device */
	RT2880_REG(RT2880_SPI_ARB_REG) = SPIARB_ARB_EN | (SPIARB_SPI1_ACTIVE_MODE <<1) | SPIARB_SPI0_ACTIVE_MODE;
        RT2880_REG(RT2880_SPI0_CTL_REG) = (~SPIARB_SPI0_ACTIVE_MODE)&0x1;     //disable first
        RT2880_REG(RT2880_SPI1_CTL_REG) = (~SPIARB_SPI1_ACTIVE_MODE)&0x1;     //disable first
#ifdef CFG_ENV_IS_IN_SPI
	spic_init();
#endif
#endif
}

void spi_vtss_cmd(cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	u8 blk, subblk, cmd, addr;
	u32 value;

	//insufficient arguments
	if (argc < 5)
		printf("Usage:\n%s\n use \"help spicmd\" for detail!\n", cmdtp->usage);

	blk = (u8)simple_strtoul(argv[2], NULL, 16);
	subblk = (u8)simple_strtoul(argv[3], NULL, 16);
	addr = (u8)simple_strtoul(argv[4], NULL, 16);

	//block, sub-bluck out of range
	if (blk == 0 || blk > 7 || subblk > 7)
		printf("Usage:\n%s\n use \"help spicmd\" for detail!\n", cmdtp->usage);

	cmd = (u8)((blk << 5) | subblk);

	if (!strncmp(argv[1], "read", 5)) {
		spi_master_init();
		spi_write(cmd);
		spi_write(addr);
		spi_read(); //dummy byte
		spi_read(); //dummy byte
		value = spi_read32();
		printf("read %d:%d:%x = 0x%x\n", blk, subblk, addr, value);
	}
	else if (!strncmp(argv[1], "write", 6)) {
		cmd |= 0x10; //set the write bit
		value = (u32)simple_strtoul(argv[5], NULL, 16);
		spi_master_init();
		spi_write(cmd);
		spi_write(addr);
		spi_write32(value);
		printf("write %d:%d:%x = 0x%x\n", blk, subblk, addr, value);
	}
	else
		printf("Usage:\n%s\n use \"help spicmd\" for detail!\n", cmdtp->usage);
}

int rt2880_spi_toolkit(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (!strncmp(argv[1], "eeprom", 7)) {
		spi_eeprom_cmd(cmdtp, argc - 1, argv + 1);
	}
	else if (!strncmp(argv[1], "vtss", 5)) {
		spi_vtss_cmd(cmdtp, argc - 1, argv + 1);
	}
	else
		printf("Usage:\n%s\n use \"help spicmd\" for detail!\n", cmdtp->usage);
	return 0;
}

U_BOOT_CMD(
	spicmd,	7,	1, 	rt2880_spi_toolkit,
	"spicmd	- read/write data from/to eeprom or vtss\n",
	"spicmd usage:\n"
	"  spicmd eeprom read [address]\n"
	"  spicmd eeprom write [size] [address] [value]\n"
	"  spicmd eeprom dump\n"
	"  spicmd vtss read [block] [sub-block] [address]\n"
	"  spicmd vtss write [block] [sub-block] [address] [value]\n"
	"    NOTE: size is 1, 2, 4 bytes only, address and value are in hex\n"
);

#endif

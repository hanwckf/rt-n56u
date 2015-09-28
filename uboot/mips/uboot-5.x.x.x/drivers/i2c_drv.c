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

#if (CONFIG_COMMANDS & CFG_CMD_I2C) 

#if defined (RT6855A_ASIC_BOARD) || defined (RT6855A_FPGA_BOARD)
#define BBU_I2C
#endif
#define	RT2880_I2C_DEVADDR_STR		"devaddr"	/* Dump Content Command Prompt    */
#define	RT2880_I2C_DUMP_STR		"dump"	/* Dump Content Command Prompt    */
#define	RT2880_I2C_READ_STR		"read"	/* I2C read operation */
#define	RT2880_I2C_WRITE_STR		"write"	/* I2C read operation */

#define RT2880_I2C_DUMP        2
#define RT2880_I2C_READ        3
#define RT2880_I2C_DEVADDR     4
#define RT2880_I2C_WRITE       5

/*---------------------------------------------------------------------*/
/* Symbol & Macro Definitions                                          */
/*---------------------------------------------------------------------*/

#define RT2880_REG(x)			(*((volatile u32 *)(x)))

#define RT2880_I2C_CONFIG_REG		(RALINK_I2C_BASE+0x00)
#define RT2880_I2C_CLKDIV_REG		(RALINK_I2C_BASE+0x04)
#define RT2880_I2C_DEVADDR_REG		(RALINK_I2C_BASE+0x08)
#define RT2880_I2C_ADDR_REG		(RALINK_I2C_BASE+0x0C)
#define RT2880_I2C_DATAOUT_REG	 	(RALINK_I2C_BASE+0x10)
#define RT2880_I2C_DATAIN_REG  		(RALINK_I2C_BASE+0x14)
#define RT2880_I2C_STATUS_REG  		(RALINK_I2C_BASE+0x18)
#define RT2880_I2C_STARTXFR_REG		(RALINK_I2C_BASE+0x1C)
#define RT2880_I2C_BYTECNT_REG		(RALINK_I2C_BASE+0x20)
#define RT2880_I2C_SM0_IS_AUTOMODE	(RALINK_I2C_BASE+0x28)
#define RT2880_I2C_SM0CTL0		(RALINK_I2C_BASE+0x40)

/* I2C_CFG register bit field */
#define I2C_CFG_ADDRLEN_8				(7<<5)	/* 8 bits */
#define I2C_CFG_DEVADLEN_7				(6<<2)	/* 7 bits */
#define I2C_CFG_ADDRDIS					(1<<1)	/* disable address transmission*/
#define I2C_CFG_DEVADDIS				(1<<0)	/* disable evice address transmission */


#define IS_BUSY		(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x01)
#define IS_SDOEMPTY	(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x02)
#define IS_DATARDY	(RT2880_REG(RT2880_I2C_STATUS_REG) & 0x04)

/*
 *  * max SCLK : 400 KHz
 *   * CLKDIV < I2C_CLK / SCLK = I2C_CLK / 0.4
 *    */
#define CLKDIV_VALUE    333

#define i2c_busy_loop		(CLKDIV_VALUE*30)
#define max_ee_busy_loop	(CLKDIV_VALUE*25)
						  

/* 
 * AT24C01A/02/04/08A/16A (1K, 2K, 4K, 8K, 16K) 
 *	-- address : 8-bits
 * AT24C512 (512K)
 *  -- address : two 8-bits
 */    
#if defined (MT7621_FPGA_BOARD)
//MT7621 FPGA board use AT24C64 EEPROM (need 2bytes word address support)
//#define CONFIG_EEPROM_ADDRESS_BYTES  2
//connected to PCI-E/USB3 DTB
#define CONFIG_EEPROM_ADDRESS_BYTES  1
#endif

#if (CONFIG_EEPROM_ADDRESS_BYTES == 2)
#define ADDRESS_BYTES	2
#else
#define ADDRESS_BYTES	1
#endif

/* 
 * sequential reads
 * because BYTECNT REG max 64 (6-bits)
 * , max READ_BLOCK is 64 
 */
#define READ_BLOCK		16

/*
 * AT24C01A/02 (1K, 2K)  have 8-byte Page
 */
#define WRITE_BLOCK		8


/*
 * ATMEL AT25XXXX Serial EEPROM 
 * access type
 */

/* Instruction codes */
#define READ_CMD	0x01
#define WRITE_CMD	0x00


#define I2C_CFG_DEFAULT			(I2C_CFG_ADDRLEN_8  | \
								 I2C_CFG_DEVADLEN_7 | \
								 I2C_CFG_ADDRDIS)


/*---------------------------------------------------------------------*/
/* Prototypes of External Functions                                    */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/* Prototypes of Functions Used                                        */
/*---------------------------------------------------------------------*/
static void i2c_write(u32 address, u8 *data, u32 nbytes);
static void i2c_read(u8 *data, u32 nbytes);

void i2c_master_init(void);


/*---------------------------------------------------------------------*/
/* External Variable Definitions                                       */
/*---------------------------------------------------------------------*/
u32 i2c_devaddr = 0x50;



/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	i2c_master_init                                         */
/*    INPUTS: None                                                      */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): Initialize I2C block to desired state                     */
/*                                                                      */
/*----------------------------------------------------------------------*/
void i2c_master_init(void)
{
#ifndef BBU_I2C
	/* reset i2c block */
	u32 val = RT2880_REG(RT2880_RSTCTRL_REG);
	val = val | RALINK_I2C_RST;
	RT2880_REG(RT2880_RSTCTRL_REG) = val;

	val = val & ~(RALINK_I2C_RST);
	RT2880_REG(RT2880_RSTCTRL_REG) = val;
	udelay(500);
#endif
	
	RT2880_REG(RT2880_I2C_CONFIG_REG) = I2C_CFG_DEFAULT;

#if defined (MT7621_ASIC_BOARD) || defined (MT7621_FPGA_BOARD)
	val = 1 << 31; // the output is pulled hight by SIF master 0
	val |= 1 << 28; // allow triggered in VSYNC pulse
	val |= CLKDIV_VALUE << 16; //clk div
	val |= 1 << 6; // output H when SIF master 0 is in WAIT state
	val |= 1 << 1; // Enable SIF master 0
	RT2880_REG(RT2880_I2C_SM0CTL0) = val;

	RT2880_REG(RT2880_I2C_SM0_IS_AUTOMODE) = 1; //auto mode
#else
	RT2880_REG(RT2880_I2C_CLKDIV_REG) = CLKDIV_VALUE;
#endif

	/*
	 * Device Address : 
	 *
	 * ATMEL 24C152 serial EEPROM
	 *       1|0|1|0|0|A1|A2|R/W
	 *      MSB              LSB
	 * 
	 * ATMEL 24C01A/02/04/08A/16A
	 *    	MSB               LSB	  
	 * 1K/2K 1|0|1|0|A2|A1|A0|R/W
	 * 4K            A2 A1 P0
	 * 8K            A2 P1 P0
	 * 16K           P2 P1 P0 
	 *
	 * so device address needs 7 bits 
	 * if device address is 0, 
	 * write 0xA0 >> 1 into DEVADDR(max 7-bits) REG  
	 */
	RT2880_REG(RT2880_I2C_DEVADDR_REG) =  i2c_devaddr;

	/*
	 * Use Address Disabled Transfer Options
	 * because it just support 8-bits, 
	 * ATMEL eeprom needs two 8-bits address
	 */
	RT2880_REG(RT2880_I2C_ADDR_REG) = 0;
}



/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	i2c_write                                               */
/*    INPUTS: 8-bit data                                                */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): transfer 8-bit data to I2C                                */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void i2c_write(u32 address, u8 *data, u32 nbytes)
{
	int i, j;
	u32 n;

	/* two bytes data at least so NODATA = 0 */
	n = nbytes + ADDRESS_BYTES;
	RT2880_REG(RT2880_I2C_BYTECNT_REG) = n-1;
	if (ADDRESS_BYTES == 2)
		RT2880_REG(RT2880_I2C_DATAOUT_REG) = (address >> 8) & 0xFF;
	else
		RT2880_REG(RT2880_I2C_DATAOUT_REG) = address & 0xFF;

	RT2880_REG(RT2880_I2C_STARTXFR_REG) = WRITE_CMD;
	for (i=0; i<n-1; i++) {
		j = 0;
		do {
			if (IS_SDOEMPTY) {
				if (ADDRESS_BYTES == 2) {
					if (i==0) {
						RT2880_REG(RT2880_I2C_DATAOUT_REG) = address & 0xFF;
					} else {
						RT2880_REG(RT2880_I2C_DATAOUT_REG) = data[i-1];
					}								
				} else {
					RT2880_REG(RT2880_I2C_DATAOUT_REG) = data[i];
				}
 			break;
			}
		} while (++j<max_ee_busy_loop);
	}

	i = 0;
	while(IS_BUSY && i<i2c_busy_loop){
		i++;
	};
}

/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*           	i2c_read                                                */
/*    INPUTS: None                                                      */
/*   RETURNS: 8-bit data                                                */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): get 8-bit data from I2C                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/
static void i2c_read(u8 *data, u32 nbytes) 
{
	int i, j;

	RT2880_REG(RT2880_I2C_BYTECNT_REG) = nbytes-1;
	RT2880_REG(RT2880_I2C_STARTXFR_REG) = READ_CMD;
	for (i=0; i<nbytes; i++) {
		j = 0;
		do {
			if (IS_DATARDY) {
				data[i] = RT2880_REG(RT2880_I2C_DATAIN_REG);
				break;
			}
		} while(++j<max_ee_busy_loop);
	}

	i = 0;
	while(IS_BUSY && i<i2c_busy_loop){
		i++;
	};
}
static inline void random_read_block(u32 address, u8 *data)
{
	/* change page */
	if (ADDRESS_BYTES == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2c_devaddr | (page >> 1));
	}

   	/* dummy write */
   	i2c_write(address, data, 0);
	i2c_read(data, READ_BLOCK);	
}

static inline u8 random_read_one_byte(u32 address)
{	
	u8 data;

	/* change page */
	if (ADDRESS_BYTES == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2c_devaddr | (page >> 1));
	}

   	/* dummy write */
	i2c_write(address, &data, 0);
	i2c_read(&data, 1);
	return (data);
}

void i2c_eeprom_read(u32 address, u8 *data, u32 nbytes)
{
	int i;
	int nblock = nbytes / READ_BLOCK;
	int rem = nbytes % READ_BLOCK;

	for (i=0; i<nblock; i++) {
		random_read_block(address+i*READ_BLOCK, &data[i*READ_BLOCK]);
	}

	if (rem) {
		int offset = nblock*READ_BLOCK;
		for (i=0; i<rem; i++) {
			data[offset+i] = random_read_one_byte(address+offset+i);
		}		
	}
}


void i2c_eeprom_read_one(u32 address, u8 *data, u32 nbytes)
{
	int i;

	for (i=0; i<nbytes; i++) {
		data[i] = random_read_one_byte(address+i);
	}
}



static inline void random_write_block(u32 address, u8 *data)
{
	/* change page */
	if (ADDRESS_BYTES == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2c_devaddr | (page >> 1));
	}


	i2c_write(address, data, WRITE_BLOCK);
	udelay(5000);
}

static inline void random_write_one_byte(u32 address, u8 *data)
{	
	/* change page */
	if (ADDRESS_BYTES == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2c_devaddr | (page >> 1));
	}

	i2c_write(address, data, 1);
	udelay(5000);
}

void i2c_eeprom_write(u32 address, u8 *data, u32 nbytes)
{
	int i;
	int nblock = nbytes / WRITE_BLOCK;
	int rem = nbytes % WRITE_BLOCK;

	for (i=0; i<nblock; i++) {
		random_write_block(address+i*WRITE_BLOCK, &data[i*WRITE_BLOCK]);
	}

	if (rem) {
		int offset = nblock*WRITE_BLOCK;

		for (i=0; i<rem; i++) {
			random_write_one_byte(address+offset+i, &data[offset+i]);
		}		
	}
}



void i2c_read_config(char *data, unsigned int len)
{
	i2c_master_init();
	i2c_eeprom_read(0, data, len);
}

void i2c_eeprom_dump(void)
{
	u32 a;
	u8 v;

	i2c_master_init();
	for (a = 0; a < 128; a++) {
		if (a % 16 == 0)
			printf("%4x : ", a);
		v = random_read_one_byte(a);
		printf("%02x ", v);
		if (a % 16 == 15)
			printf("\n");
	}
}

int rt2880_i2c_toolkit(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int chk_match, size;
	ulong addr, value;
	u16 address;

#ifndef BBU_I2C
	/* configure i2c to normal mode */
#if defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD)
	RT2880_REG(RT2880_GPIOMODE_REG) &= ~(1 << 2);
#elif defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
	RT2880_REG(RT2880_GPIOMODE_REG) &= ~(1 << 16);
#else
	RT2880_REG(RT2880_GPIOMODE_REG) &= ~1;
#endif
#endif

	switch (argc) {
		case RT2880_I2C_DUMP:
			chk_match = strcmp(argv[1], RT2880_I2C_DUMP_STR);
			if (chk_match != 0) {
				printf("Usage:\n%s\n", cmdtp->usage);
				return 1;
			}
			i2c_eeprom_dump();
			break;
		case RT2880_I2C_READ:
			chk_match = strcmp(argv[1], RT2880_I2C_READ_STR);
			if (chk_match != 0) {
				printf("Usage:\n%s\n", cmdtp->usage);
				return 1;
			}
			addr = simple_strtoul(argv[2], NULL, 16);
			address = addr;
			i2c_master_init();
			i2c_eeprom_read(addr, (u8*)&value, 4);
			printf("0x%04x : 0x%04x\n", address, value);
			break;
		case RT2880_I2C_WRITE:
			chk_match = strcmp(argv[1], RT2880_I2C_WRITE_STR);
			if (chk_match != 0) {
				printf("Usage:\n%s\n", cmdtp->usage);
				return 1;
			}
			size = simple_strtoul(argv[2], NULL, 16);
			addr = simple_strtoul(argv[3], NULL, 16);
			value = simple_strtoul(argv[4], NULL, 16);
			i2c_master_init();
			address = addr;
			i2c_eeprom_write(address, (u8*)&value, size);
			printf("0x%08x: 0x%08x in %d bytes\n", address, value, size);
			break;
		case RT2880_I2C_DEVADDR:
			chk_match = strcmp(argv[2],  RT2880_I2C_DEVADDR_STR);
			if (chk_match != 0) {
				printf("Usage:\n%s\n", cmdtp->usage);
				return 1;
			}
			i2c_devaddr = simple_strtoul(argv[3], NULL, 16);
			break;
		default:
			printf("Usage:\n%s\n use \"help i2ccmd\" to get more detail!\n", cmdtp->usage);
	}
	return 0;
}

U_BOOT_CMD(
	i2ccmd,	5,	1, 	rt2880_i2c_toolkit,
	"i2ccmd	- read/write data to eeprom via I2C Interface\n",
	"i2ccmd read/write eeprom_address data(if write)\n"
	"i2ccmd format:\n"
	"  i2ccmd set devaddr [addr in hex]\n"
	"  i2ccmd read [offset in hex]\n"
	"  i2ccmd write [size] [offset] [value]\n"
	"  i2ccmd dump\n"
	"NOTE -- size is 1, 2, 4 bytes only, offset and value are in hex\n"
);

#endif

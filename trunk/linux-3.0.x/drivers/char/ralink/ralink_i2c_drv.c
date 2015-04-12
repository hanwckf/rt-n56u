/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 ***************************************************************************
 *
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
#include <asm/system.h>
#endif
#include <linux/wireless.h>

#include <ralink/ralink_gpio.h>
#include <ralink/ralink_i2c_drv.h>

#if defined(CONFIG_MTK_NFC_SUPPORT)
  #if defined(CONFIG_RALINK_RT3883)
    #define MT6605_GPIO_IND         9  // MT6605_IRQ
    #define MT6605_GPIO_VEN         13 // MT6605_VEN
    #define MT6605_GPIO_RESET       11 // MT6605_RESET
  #elif defined(CONFIG_RALINK_MT7621)
    #define MT6605_GPIO_IND         10 // MT6605_IRQ
    #define MT6605_GPIO_VEN         9  // MT6605_VEN
    #define MT6605_GPIO_RESET       6  // MT6605_RESET
  #else
    #error "chip is not support"
  #endif
#endif

#if defined(CONFIG_MTK_NFC_MT6605_SIM)
extern void mt6605_sim(void);
#endif

int i2cdrv_major = 218;
unsigned long i2cdrv_addr = ATMEL_ADDR;
unsigned long address_bytes= 2;
unsigned long clkdiv_value = CLKDIV_VALUE;

unsigned long switch_address_bytes(unsigned long addr_bytes)
{
	address_bytes = addr_bytes;
	//printk("I2C controller address bytes is %x\n", addr_bytes);
	return address_bytes;
}

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
	u32 i;

	/* reset i2c block */
	i = RT2880_REG(RT2880_RSTCTRL_REG);
	RT2880_REG(RT2880_RSTCTRL_REG) = (i | RALINK_I2C_RST);
	RT2880_REG(RT2880_RSTCTRL_REG) = (i & ~(RALINK_I2C_RST));
	udelay(500);

	RT2880_REG(RT2880_I2C_CONFIG_REG) = I2C_CFG_DEFAULT;

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	i = 1 << 31; // the output is pulled hight by SIF master 0
	i |= 1 << 28; // allow triggered in VSYNC pulse
	i |= clkdiv_value << 16; //clk div
	i |= 1 << 6; // output H when SIF master 0 is in WAIT state
	i |= 1 << 1; // Enable SIF master 0
	RT2880_REG(RT2880_I2C_SM0CTL0) = i;
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
	RT2880_REG(RT2880_I2C_DEVADDR_REG) = i2cdrv_addr;

	/*
	 * Use Address Disabled Transfer Options
	 * because it just support 8-bits, 
	 * ATMEL eeprom needs two 8-bits address
	 */
	RT2880_REG(RT2880_I2C_ADDR_REG) = 0;
}


/*----------------------------------------------------------------------*/
/*   Function                                                           */
/*       i2c_WM8751_write                                               */
/*    INPUTS: 8-bit data                                                */
/*   RETURNS: None                                                      */
/*   OUTPUTS: None                                                      */
/*   NOTE(S): transfer 8-bit data to I2C                                */
/*                                                                      */
/*----------------------------------------------------------------------*/
#if defined (CONFIG_RALINK_I2S) || defined (CONFIG_RALINK_I2S_MODULE)
void i2c_WM8751_write(u32 address, u32 data)
{
	int i, j;
	unsigned long old_addr = i2cdrv_addr;
	
#if defined(CONFIG_SND_SOC_WM8960) || ((!defined(CONFIG_SND_RALINK_SOC)) && defined(CONFIG_I2S_WM8960))
	i2cdrv_addr = WM8960_ADDR;
#elif defined(CONFIG_SND_SOC_WM8750) || ((!defined(CONFIG_SND_RALINK_SOC)) && defined(CONFIG_I2S_WM8750)) || ((!defined(CONFIG_SND_RALINK_SOC)) && defined(CONFIG_I2S_WM8751))
	i2cdrv_addr = WM8751_ADDR;
#else
	i2cdrv_addr = WM8960_ADDR;
#endif

	i2c_master_init();

	/* two bytes data at least so NODATA = 0 */

	RT2880_REG(RT2880_I2C_BYTECNT_REG) = 1;
	
	RT2880_REG(RT2880_I2C_DATAOUT_REG) = (address<<1)|(0x01&(data>>8));

	RT2880_REG(RT2880_I2C_STARTXFR_REG) = WRITE_CMD;

	j = 0;
	do {
		if (IS_SDOEMPTY) {	
			RT2880_REG(RT2880_I2C_DATAOUT_REG) = (data&0x0FF);	
			break;
		}
	} while (++j<max_ee_busy_loop);
	
	i = 0;
	while(IS_BUSY && i<i2c_busy_loop){
		i++;
	};
	i2cdrv_addr = old_addr;
}
EXPORT_SYMBOL(i2c_WM8751_write);
#endif

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
	n = nbytes + address_bytes;
	RT2880_REG(RT2880_I2C_BYTECNT_REG) = n-1;
	if (address_bytes == 2)
		RT2880_REG(RT2880_I2C_DATAOUT_REG) = (address >> 8) & 0xFF;
	else
		RT2880_REG(RT2880_I2C_DATAOUT_REG) = address & 0xFF;

	RT2880_REG(RT2880_I2C_STARTXFR_REG) = WRITE_CMD;
	for (i=0; i<n-1; i++) {
		j = 0;
		do {
			if (IS_SDOEMPTY) {
				if (address_bytes == 2) {
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
	if (address_bytes == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2cdrv_addr | (page>>1));
	}

   	/* dummy write */
   	i2c_write(address, data, 0);
	i2c_read(data, READ_BLOCK);	
}

u8 random_read_one_byte(u32 address)
{	
	u8 data;
#ifdef EEPROM_1B_ADDRESS_2KB_SUPPORT
	/* change page */
	if (address_bytes == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2cdrv_addr | (page>>1));
	}
#endif

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
	if (address_bytes == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2cdrv_addr | (page>>1));
	}


	i2c_write(address, data, WRITE_BLOCK);
	udelay(5000);
}

void random_write_one_byte(u32 address, u8 *data)
{	
#ifdef EEPROM_1B_ADDRESS_2KB_SUPPORT
	/* change page */
	if (address_bytes == 1) {
		int page;
		
		page = ((address >> 8) & 0x7) << 1;
		/* device id always 0 */
		RT2880_REG(RT2880_I2C_DEVADDR_REG) = (i2cdrv_addr | (page>>1));
	}
#endif

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
			printk("%4x : ", a);
		v = random_read_one_byte(a);
		printk("%02x ", v);
		if (a % 16 == 15)
			printk("\n");
	}
}

#if defined(CONFIG_MTK_NFC_SUPPORT)

#define MAX_BLOCK       64
#define MAX_LOOP        1000
#define GPIO_INPUT      0
#define GPIO_OUTPUT     1

#if defined (CONFIG_RALINK_RT3883)
void MT6605_gpio_init(u8 num, u8 dir)
{
        int reg = 0;
        if ((num >= 25) && (num <= 31))
        {
                reg = RT2880_REG(RALINK_SYSCTL_BASE+0x14);
                reg &= ~(0x3<<1); // GPIO 2, 3~7 as GPIO mode
                RT2880_REG(RALINK_SYSCTL_BASE+0x14) = reg;

                //set direction
                reg = RT2880_REG(RALINK_PIO_BASE+0x4c);
                if (dir == GPIO_INPUT)
                        reg &= ~(1<<(num-24)); // input
                else
                        reg |= (1<<(num-24)); // output
                RT2880_REG(RALINK_PIO_BASE+0x4c) = reg;
        }
        else if ((num >= 7) && (num <= 14))
        {
                reg = RT2880_REG(RALINK_SYSCTL_BASE+0x60);
                reg |= (0x7<<2); // UARTF as GPIO mode
                RT2880_REG(RALINK_SYSCTL_BASE+0x60) = reg;

                //set direction = input
                reg = RT2880_REG(RALINK_PIO_BASE+0x24);
                if (dir == GPIO_INPUT)
                        reg &= ~(1<<num); // input
                else
                        reg |= (1<<num); // output
                RT2880_REG(RALINK_PIO_BASE+0x24) = reg;
        }
        else
                printk("GPIO num not support\n");
}

int MT6605_gpio_read(u8 num)
{
        int reg = 0;

        if ((num >= 25) && (num <= 31))
        {
                reg = RT2880_REG(0x48+RALINK_PIO_BASE) & (1<<(num-24));

        }
        else if ((num >= 7) && (num <= 14))
        {
                reg = RT2880_REG(0x20+RALINK_PIO_BASE) & (1<<(num));
        }
        else
                printk("GPIO num not support\n");

        return reg;
}

void MT6605_gpio_write(u8 num, u8 val)
{
        int reg = 0;
        if ((num >= 25) && (num <= 31))
        {
                reg = RT2880_REG(0x48+RALINK_PIO_BASE);
                if (val)
                        reg |= (1<<(num-24));
                else
                        reg &= ~(1<<(num-24));
                RT2880_REG(0x48+RALINK_PIO_BASE) = reg;
        }
        else if ((num >= 7) && (num <= 14))
        {
                reg = RT2880_REG(0x20+RALINK_PIO_BASE);
                if (val)
                        reg |= (1<<(num));
                else
                        reg &= ~(1<<(num));
                RT2880_REG(0x20+RALINK_PIO_BASE) = reg;
        }
        else
                printk("GPIO num not support\n");
}
#elif defined(CONFIG_RALINK_MT7621)
void MT6605_gpio_init(u8 num, u8 dir)
{
	int reg = 0;

	reg = RT2880_REG(RALINK_SYSCTL_BASE+0x60);
	reg &= ~(0xf << 3);
	reg |= (0x5 << 3); // UART2 and UART3
	RT2880_REG(RALINK_SYSCTL_BASE+0x60) = reg;

	//set direction = input
	reg = RT2880_REG(RALINK_REG_PIODIR);
	if (dir == GPIO_INPUT)
		reg &= ~(1<<num); // input
	else
		reg |= (1<<num); // output
	RT2880_REG(RALINK_REG_PIODIR) = reg;
}

int MT6605_gpio_read(u8 num)
{
	return RT2880_REG(RALINK_REG_PIODATA) & (1<<(num));
}

void MT6605_gpio_write(u8 num, u8 val)
{
	int reg;

	reg = RT2880_REG(RALINK_REG_PIODATA);
	if (val)
		reg |= (1<<(num));
	else
		reg &= ~(1<<(num));
	RT2880_REG(RALINK_REG_PIODATA) = reg;
 
}
#else
#error "Chip not Support NFC"
#endif

EXPORT_SYMBOL(MT6605_gpio_read);
EXPORT_SYMBOL(MT6605_gpio_init);
EXPORT_SYMBOL(MT6605_gpio_write);

void i2c_read_MT6605(char *data, unsigned int len)
{
        int i, j;
        unsigned long old_addr = i2cdrv_addr;
        int nblock = len / MAX_BLOCK;
        int rem = len % MAX_BLOCK;
        int reg;

        for (j = 0; j < MAX_LOOP; j++)
        {
                reg = MT6605_gpio_read(MT6605_GPIO_IND); // RT2880_REG(0x48+RALINK_PIO_BASE) & (1<<2);
                if (reg)
                        break;
                udelay(1);
        }

        if (j >= MAX_LOOP)
        {
                printk("GPIO Read Timeout\n");
                return;
        }


        i2cdrv_addr = (0x28 >> 0);
        i2c_master_init();

        for (i=0; i<len; i++) {
                if ((i & (MAX_BLOCK-1)) == 0)
                {
                        if (nblock > 0)
                        {
                                RT2880_REG(RT2880_I2C_BYTECNT_REG) = MAX_BLOCK - 1;
                                nblock--;
                        }
                        else
                                RT2880_REG(RT2880_I2C_BYTECNT_REG) = rem - 1;

                        RT2880_REG(RT2880_I2C_STARTXFR_REG) = READ_CMD;
                }

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

        i2cdrv_addr = old_addr;

}

void i2c_write_MT6605(char *data, unsigned int len)
{
        int i, j;
        unsigned long old_addr = i2cdrv_addr;
        int nblock = len / MAX_BLOCK;
        int rem = len % MAX_BLOCK;
        int reg;

        for (j = 0; j < MAX_LOOP; j++)
        {
                reg = MT6605_gpio_read(MT6605_GPIO_IND); // RT2880_REG(0x48+RALINK_PIO_BASE) & (1<<2);
                if (!reg)
                        break;
                udelay(1);
        }

        if (j >= MAX_LOOP)
        {
                printk("GPIO Write Timeout\n");
                return;
        }


        i2cdrv_addr = (0x28 >> 0);
        i2c_master_init();


        for (i=0; i<len; i++) {

                if ((i & (MAX_BLOCK-1)) == 0)
                {

                        j = 0;
                        while(IS_BUSY && j<i2c_busy_loop){
                                j++;
                        };


                        if (nblock > 0)
                        {
                                RT2880_REG(RT2880_I2C_BYTECNT_REG) = MAX_BLOCK - 1;
                                nblock--;
                        }
                        else
                        {
                                RT2880_REG(RT2880_I2C_BYTECNT_REG) = rem - 1;
                        }
                }

                j = 0;
                do {
                        if (IS_SDOEMPTY) {
                                RT2880_REG(RT2880_I2C_DATAOUT_REG) = data[i];
                                break;
                        }
                } while (++j<max_ee_busy_loop);

                if ((i & (MAX_BLOCK-1)) == 0)
                {
                        RT2880_REG(RT2880_I2C_STARTXFR_REG) = WRITE_CMD;
                }

        }

        i = 0;
        while(IS_BUSY && i<i2c_busy_loop){
                i++;
        };

        i2cdrv_addr = old_addr;
}

EXPORT_SYMBOL(i2c_read_MT6605);
EXPORT_SYMBOL(i2c_write_MT6605);
#endif

long i2cdrv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	//unsigned char w_byte[4];
	unsigned int address, size;
	u8 value, *tmp;
	I2C_WRITE *s_i2c_write;

	switch (cmd) {
	case RT2880_PCIE_PHY_READ:
		value = 0; address = 0;
		address = (unsigned int)arg;
		i2c_master_init();
		value = random_read_one_byte(address);
		printk("0x%04x : 0x%x\n", address, (unsigned char)value);
		break;
	case RT2880_PCIE_PHY_WRITE:
		s_i2c_write = (I2C_WRITE*)arg;
		address = s_i2c_write->address;
		value   = s_i2c_write->value;
		size    = s_i2c_write->size;
		tmp = &value;
		i2c_master_init();
		//random_write_one_byte(address, &value);
		i2c_write((unsigned long)address, tmp, 4);
		break;
	case RT2880_I2C_DUMP:
		i2c_eeprom_dump();
		break;
	case RT2880_I2C_READ:
		value = 0; address = 0;
		address = (unsigned int)arg;
#if defined(CONFIG_MTK_NFC_MT6605_SIM)
		// to test NFC, build i2ccmd, use the following command to test
		// i2ccmd read 4444
		// i2ccmd read 3333
		// i2ccmd read 2222
		if (address == 0x2222)
		{
			mt6605_sim();
		}
		else if (address == 0x3333)
		{
                        MT6605_gpio_write(MT6605_GPIO_VEN, 0);
                        MT6605_gpio_write(MT6605_GPIO_RESET, 1);
		}
		else if (address == 0x4444)
		{
                        MT6605_gpio_write(MT6605_GPIO_RESET, 0);
                        MT6605_gpio_write(MT6605_GPIO_VEN, 1);
		}
#else
		i2c_master_init();
		i2c_eeprom_read(address, (unsigned char*)&value, 4);
		printk("0x%04x : 0x%08x\n", address, (unsigned int)value);
#endif
		break;
	case RT2880_I2C_WRITE:
		s_i2c_write = (I2C_WRITE*)arg;
		address = s_i2c_write->address;
		value   = s_i2c_write->value;
		size    = s_i2c_write->size;
		i2c_master_init();
		i2c_eeprom_write(address, (unsigned char*)&value, size);
#if 0
		memcpy(w_byte, (unsigned char*)&value, 4);
		if ( size == 4) {
			i2c_eeprom_write(address, w_byte[0], 1);
			i2c_eeprom_write(address+1, w_byte[1], 1 );
			i2c_eeprom_write(address+2, w_byte[2], 1 );
			i2c_eeprom_write(address+3, w_byte[3], 1 );
		} else if (size == 2) {
			i2c_eeprom_write(address, w_byte[0], 1);
			i2c_eeprom_write(address+1, w_byte[1], 1 );
		} else if (size == 1) {
			i2c_eeprom_write(address, w_byte[0], 1);
		} else {
			printk("i2c_drv -- size error, %d\n", size);
			return 0;
		}
#endif
		break;
	case RT2880_I2C_SET_ADDR:
		i2cdrv_addr = (unsigned long)arg;
		break;
	case RT2880_I2C_SET_ADDR_BYTES:
		value = switch_address_bytes( (unsigned long)arg);
		printk("i2c addr bytes = %x\n", value);
		break;
	case RT2880_I2C_SET_CLKDIV:
		clkdiv_value = 40*1000/(unsigned long)arg;
		printk("i2c clkdiv = %d\n", clkdiv_value);
		break;
	default :
		printk("i2c_drv: command format error\n");
	}

	return 0;
}

static int i2cdrv_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static int i2cdrv_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

static const struct file_operations i2cdrv_fops =
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= i2cdrv_ioctl,
	.open		= i2cdrv_open,
	.release	= i2cdrv_release,
};

int __init i2cdrv_init(void)
{
	int result = register_chrdev(i2cdrv_major, I2C_DEV_NAME, &i2cdrv_fops);
	if (result < 0) {
		printk(KERN_WARNING "i2c_drv: can't get major %d\n",i2cdrv_major);
		return result;
	}

	if (i2cdrv_major == 0) {
		i2cdrv_major = result; /* dynamic */
	}

	/* configure i2c to normal mode */
	RT2880_REG(RALINK_SYSCTL_BASE + 0x60) &= ~RALINK_GPIOMODE_I2C;

#if defined(CONFIG_MTK_NFC_SUPPORT)
	MT6605_gpio_init(MT6605_GPIO_IND, GPIO_INPUT); // IND
	MT6605_gpio_init(MT6605_GPIO_VEN, GPIO_OUTPUT); // VEN
	MT6605_gpio_init(MT6605_GPIO_RESET, GPIO_OUTPUT); // Reset

	MT6605_gpio_write(MT6605_GPIO_RESET, 0);
	MT6605_gpio_write(MT6605_GPIO_VEN, 1);
#endif

	printk("Ralink I2C driver initialized.\n");

	return 0;
}

void __exit i2cdrv_exit(void)
{
	unregister_chrdev(i2cdrv_major, I2C_DEV_NAME);
}

module_init(i2cdrv_init);
module_exit(i2cdrv_exit);

module_param (i2cdrv_major, int, 0);

MODULE_DESCRIPTION("Ralink I2C Driver");
MODULE_LICENSE("GPL");

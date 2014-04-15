#include <common.h>
#include <command.h>
#include <rt_mmap.h>
#include <ralink_gpio.h>

#include "ralink_smi.h"

#define DELAY			3
#define CLK_DURATION(clk)	udelay(clk)
#define _SMI_ACK_RESPONSE(ok)	{ /*if (!(ok)) return RT_ERR_FAILED; */}
#define ack_timer		5

const int smi_SCK = 2;	/* GPIO used for SMI Clock generation */	/* GPIO2 */
const int smi_SDA = 1;	/* GPIO used for SMI Data signal */		/* GPIO1 */


static int
ralink_gpio_write_bit(int idx, int value)
{
	unsigned long tmp;

	if (idx < 0 || idx >= RALINK_GPIO_NUMBER)
		return -1;

	if (idx <= 23) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		if (value & 1L)
			tmp |= (1L << idx);
		else
			tmp &= ~(1L << idx);
		*(volatile u32 *)(RALINK_REG_PIODATA) = cpu_to_le32(tmp);
	}
	else if (idx <= 39) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		if (value & 1L)
			tmp |= (1L << (idx-24));
		else
			tmp &= ~(1L << (idx-24));
		*(volatile u32 *)(RALINK_REG_PIO3924DATA) = cpu_to_le32(tmp);
	}
	else {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
		if (value & 1L)
			tmp |= (1L << (idx-40));
		else
			tmp &= ~(1L << (idx-40));
		*(volatile u32 *)(RALINK_REG_PIO5140DATA) = cpu_to_le32(tmp);
	}

	return 0;
}

static int
ralink_gpio_read_bit(int idx, int *value)
{
	unsigned long tmp;

	*value = 0;

	if (idx < 0 || idx >= RALINK_GPIO_NUMBER)
		return -1;

	if (idx <= 23)
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
	else if (idx <= 39)
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
	else
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));

	if (idx <= 23)
		tmp = (tmp >> idx) & 1L;
	else if (idx <= 39)
		tmp = (tmp >> (idx-24)) & 1L;
	else
		tmp = (tmp >> (idx-40)) & 1L;

	*value = tmp;

	return tmp;
}

static int
ralink_initGpioPin(unsigned int idx, int dir)
{
	unsigned long tmp;

	if (idx < 0 || idx >= RALINK_GPIO_NUMBER)
		return -1;

	if (dir == GPIO_DIR_OUT)
	{
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp |= (1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp |= (1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp |= (1L << (idx-40));
		}
	}
	else if (dir == GPIO_DIR_IN)
	{
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp &= ~(1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp &= ~(1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp &= ~(1L << (idx-40));
		}
	}
	else
		return -1;

	if (idx <= 23)
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	else if (idx <= 39)
		*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tmp);
	else
		*(volatile u32 *)(RALINK_REG_PIO5140DIR) = cpu_to_le32(tmp);

	return 0;
}

static void
_smi_start(void)
{
    /* change GPIO pin to Output only */
    ralink_initGpioPin(smi_SDA, GPIO_DIR_OUT);
    ralink_initGpioPin(smi_SCK, GPIO_DIR_OUT);

    /* Initial state: SCK: 0, SDA: 1 */
    ralink_gpio_write_bit(smi_SCK, 0);
    ralink_gpio_write_bit(smi_SDA, 1);
    CLK_DURATION(DELAY);

    /* CLK 1: 0 -> 1, 1 -> 0 */
    ralink_gpio_write_bit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 0);
    CLK_DURATION(DELAY);

    /* CLK 2: */
    ralink_gpio_write_bit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SDA, 0);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 0);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SDA, 1);
}

static void
_smi_writeBit(uint16 signal, uint32 bitLen)
{
    for( ; bitLen > 0; bitLen--)
    {
	CLK_DURATION(DELAY);

	/* prepare data */
	if ( signal & (1<<(bitLen-1)) ) 
	    ralink_gpio_write_bit(smi_SDA, 1);    
	else 
	    ralink_gpio_write_bit(smi_SDA, 0);    
	CLK_DURATION(DELAY);

	/* clocking */
	ralink_gpio_write_bit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	ralink_gpio_write_bit(smi_SCK, 0);
    }
}

static void
_smi_readBit(uint32 bitLen, uint32 *rData) 
{
    uint32 u;

    /* change GPIO pin to Input only */
    ralink_initGpioPin(smi_SDA, GPIO_DIR_IN);

    for (*rData = 0; bitLen > 0; bitLen--)
    {
	CLK_DURATION(DELAY);

	/* clocking */
	ralink_gpio_write_bit(smi_SCK, 1);
	CLK_DURATION(DELAY);
	ralink_gpio_read_bit(smi_SDA, &u);
	ralink_gpio_write_bit(smi_SCK, 0);

	*rData |= (u << (bitLen - 1));
    }

    /* change GPIO pin to Output only */
    ralink_initGpioPin(smi_SDA, GPIO_DIR_OUT);
}

static void
_smi_stop(void)
{
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SDA, 0);    
    ralink_gpio_write_bit(smi_SCK, 1);    
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SDA, 1);    
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 1);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 0);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 1);

    /* add a click */
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 0);
    CLK_DURATION(DELAY);
    ralink_gpio_write_bit(smi_SCK, 1);

    /* change GPIO pin to Output only */
    ralink_initGpioPin(smi_SDA, GPIO_DIR_IN);
    ralink_initGpioPin(smi_SCK, GPIO_DIR_IN);
}

int32 smi_read(uint32 mAddrs, uint32 *rData)
{
    uint32 rawData=0, ACK;
    uint8  con;
    uint32 ret = RT_ERR_OK;

    _smi_start();				/* Start SMI */
    _smi_writeBit(0x0b, 4);			/* CTRL code: 4'b1011 for RTL8370 */
    _smi_writeBit(0x4, 3);			/* CTRL code: 3'b100 */
    _smi_writeBit(0x1, 1);			/* 1: issue READ command */

    con = 0;
    do {
	con++;
	_smi_readBit(1, &ACK);			/* ACK for issuing READ command */
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs&0xff), 8);		/* Set reg_addr[7:0] */

    con = 0;
    do {
	con++;
	_smi_readBit(1, &ACK);			/* ACK for setting reg_addr[7:0] */
    } while ((ACK != 0) && (con < ack_timer));

    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs>>8), 8);		/* Set reg_addr[15:8] */

    con = 0;
    do {
	con++;
	_smi_readBit(1, &ACK);			/* ACK by RTL8369 */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_readBit(8, &rawData);			/* Read DATA [7:0] */
    *rData = rawData&0xff; 

    _smi_writeBit(0x00, 1);			/* ACK by CPU */
    _smi_readBit(8, &rawData);			/* Read DATA [15: 8] */
    _smi_writeBit(0x01, 1);			/* ACK by CPU */

    *rData |= (rawData<<8);

    _smi_stop();

    return ret;
}

int32 smi_write(uint32 mAddrs, uint32 rData)
{
    int8 con;
    uint32 ACK;
    uint32 ret = RT_ERR_OK;

    _smi_start();				/* Start SMI */
    _smi_writeBit(0x0b, 4);			/* CTRL code: 4'b1011 for RTL8370*/
    _smi_writeBit(0x4, 3);			/* CTRL code: 3'b100 */
    _smi_writeBit(0x0, 1);			/* 0: issue WRITE command */

    con = 0;
    do {
	con++;
	_smi_readBit(1, &ACK);			/* ACK for issuing WRITE command */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs&0xff), 8);		/* Set reg_addr[7:0] */

    con = 0;
    do {
	con++;
	_smi_readBit(1, &ACK);			/* ACK for setting reg_addr[7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit((mAddrs>>8), 8);		/* Set reg_addr[15:8] */

    con = 0;
    do {
	con++;
	_smi_readBit(1, &ACK);			/* ACK for setting reg_addr[15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit(rData&0xff, 8);		/* Write Data [7:0] out */

    con = 0;
    do {
	con++;
	_smi_readBit(1, &ACK);			/* ACK for writting data [7:0] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_writeBit(rData>>8, 8);			/* Write Data [15:8] out */

    con = 0;
    do {
	con++;
	_smi_readBit(1, &ACK);			/* ACK for writting data [15:8] */
    } while ((ACK != 0) && (con < ack_timer));
    if (ACK != 0) ret = RT_ERR_FAILED;

    _smi_stop();

    return ret;
}


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

#include <ralink/ralink_gpio.h>
#include "../../mtd/ralink/ralink_spi.h"

#define LDV_DEVNAME	"ldv0"
int ldv_major = 212;

#define ra_inl(addr)  (*(volatile unsigned int *)(addr))
#define ra_outl(addr, value)  (*(volatile unsigned int *)(addr) = (value))

#define ra_aor(addr, a_mask, o_value)  ra_outl(addr, (ra_inl(addr) & (a_mask)) | (o_value))

#define ra_and(addr, a_mask)  ra_aor(addr, a_mask, 0)
#define ra_or(addr, o_value)  ra_aor(addr, -1, o_value)

#define LINE_DRIVER_READ	0x1	// Read Single registers
#define LINE_DRIVER_WRITE	0x2	// Write
#define LINE_DRIVER_DUMP	0x3

static unsigned int spi_wait_nsec = 0;

static int spic_busy_wait(void)
{
	do {
		if ((ra_inl(RT2880_SPISTAT_REG) & 0x01) == 0)
			return 0;
	} while (spi_wait_nsec >> 1);

	printk("%s: fail \n", __func__);
	return -1;
}

int spic_init(void)
{
	 /* use normal SPI mode instead of GPIO mode */
	ra_and(RALINK_REG_GPIOMODE, ~(RALINK_GPIOMODE_SPI));

	/* reset spi block */
	ra_or(RT2880_RSTCTRL_REG, RSTCTRL_SPI_RESET);
	udelay(10);
	ra_and(RT2880_RSTCTRL_REG, ~RSTCTRL_SPI_RESET);
	udelay(10);

	ra_outl(RT2880_SPI0_CTL_REG, (~SPIARB_SPI0_ACTIVE_MODE)&0x1);

	ra_outl(RT2880_SPICFG_REG, SPICFG_MSBFIRST | SPICFG_TXCLKEDGE_FALLING | CFG_CLK_DIV | SPICFG_SPICLKPOL );

	// set idle state
	ra_outl(RT2880_SPICTL_REG, SPICTL_HIZSDO | SPICTL_SPIENA_HIGH);

	spi_wait_nsec = (8 * 1000 / (128 / CFG_CLK_DIV) ) >> 1 ;

	return 0;
}

int ldv_read(unsigned char offset)
{
	unsigned char ret = 0;
	
	// assert CS and we are already CLK normal high
	ra_and(RT2880_SPICTL_REG, ~(SPICTL_SPIENA_HIGH));

	// write command
	ra_outl(RT2880_SPIDATA_REG, offset);
	ra_or(RT2880_SPICTL_REG, SPICTL_STARTWR);
	if (spic_busy_wait()) {
		return -1;
	}

	ra_or(RT2880_SPICTL_REG, SPICTL_STARTRD);
	if (spic_busy_wait())
		return -1;
	ret = (u8) ra_inl(RT2880_SPIDATA_REG);

	// de-assert CS and
	ra_or (RT2880_SPICTL_REG, (SPICTL_SPIENA_HIGH));

	return ret;
}

int ldv_write(unsigned char offset, unsigned char value, unsigned char mask)
{
	// assert CS and we are already CLK normal high
	ra_and(RT2880_SPICTL_REG, ~(SPICTL_SPIENA_HIGH));

	// write command | 0x80
	ra_outl(RT2880_SPIDATA_REG, (0x80 | offset));
	ra_or(RT2880_SPICTL_REG, SPICTL_STARTWR);
	if (spic_busy_wait()) {
		return -1;
	}

	ra_outl(RT2880_SPIDATA_REG, mask);
	ra_or(RT2880_SPICTL_REG, SPICTL_STARTWR);
	if (spic_busy_wait()) {
		return -1;
	}

	ra_outl(RT2880_SPIDATA_REG, value);
	ra_or(RT2880_SPICTL_REG, SPICTL_STARTWR);
	if (spic_busy_wait()) {
		return -1;
	}

	// de-assert CS and
	ra_or (RT2880_SPICTL_REG, (SPICTL_SPIENA_HIGH));

	return 0;
}

int ldv_dump(void)
{
	int i, val;
	for (i = 0; i < 0x20; i++)
	{
		if ((i & 0x7) == 0)
			printk("\n");
		val = ldv_read(i);
		printk("%02x ", val);
		
	}
	printk("\n");
	return 0;
}

static long ldv_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned int *p;
	unsigned char value, offset, mask, method;

	p = (unsigned int *)arg;

	method = cmd & 0xff;
	mask = (cmd >> 8) & 0xff;

	offset = (*p >> 8) & 0xff;
	value = *p & 0xff;

	if (mask)
		mask = 0xff;

	if (offset > 0x1f)
	{
		printk("offset should be 0~ 0x1f\n");
		return 0;
	}

	if (method == LINE_DRIVER_DUMP)
	{
		ldv_dump();
	}
	else if (method == LINE_DRIVER_READ)
	{
		printk("%x\n", ldv_read(offset));
	}
	else if (method == LINE_DRIVER_WRITE)
	{
		ldv_write(offset, value, 0xff);
	}

	return 0;
}

static int ldv_open(struct inode *inode, struct file *filp)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static int ldv_release(struct inode *inode, struct file *filp)
{
	module_put(THIS_MODULE);
	return 0;
}

static const struct file_operations ldv_fops = 
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= ldv_ioctl,
	.open		= ldv_open,
	.release	= ldv_release,
};

int __init spi_ldv_init(void)
{
	int result = register_chrdev(ldv_major, LDV_DEVNAME, &ldv_fops);
	if (result < 0) {
		printk(KERN_WARNING "ps: can't get major %d\n",ldv_major);
		return result;
	}

	if (ldv_major == 0) {
		ldv_major = result; /* dynamic */
	}

	spic_init();

	return 0;
}

void __exit spi_ldv_exit(void)
{
	unregister_chrdev(ldv_major, LDV_DEVNAME);
}

module_init(spi_ldv_init);
module_exit(spi_ldv_exit);

module_param(ldv_major, int, 0);

MODULE_DESCRIPTION("Ralink SPI Line Driver");
MODULE_LICENSE("GPL");
